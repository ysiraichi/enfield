#include "enfield/Transform/Allocators/OptBMTQAllocator.h"
#include "enfield/Transform/Allocators/BMT/DefaultBMTQAllocatorImpl.h"
#include "enfield/Transform/PassCache.h"
#include "enfield/Transform/CircuitGraphBuilderPass.h"
#include "enfield/Transform/QubitRemapPass.h"
#include "enfield/Transform/Utils.h"
#include "enfield/Analysis/NodeVisitor.h"
#include "enfield/Support/ApproxTSFinder.h"
#include "enfield/Support/SimplifiedApproxTSFinder.h"
#include "enfield/Support/CommandLine.h"
#include "enfield/Support/Stats.h"
#include "enfield/Support/Defs.h"
#include "enfield/Support/Timer.h"

#include <algorithm>

using namespace efd;
using namespace opt_bmt;

using CircuitNode = CircuitGraph::CircuitNode;

static Opt<uint32_t> MaxPartialSolutions
("-bmt-max-partial", "Limits the max number of partial solutions per step.",
 std::numeric_limits<uint32_t>::max(), false);

extern Stat<double> Phase1Time;
extern Stat<double> Phase2Time;
extern Stat<double> Phase3Time;
extern Stat<uint32_t> Partitions;

// --------------------- OptBMTQAllocator:POD ------------------------
namespace efd {
namespace opt_bmt {
    struct MappingCandidate {
        Mapping m;
        uint32_t cost;
        uint32_t weight;
    };

    bool operator>(const MappingCandidate& lhs, const MappingCandidate& rhs) {
        return lhs.weight > rhs.weight;
    }
    
    struct CNodeCandidate {
        Dep dep;
        CircuitNode::Ref cNode;
        uint32_t weight;
    };

    bool operator>(const CNodeCandidate& lhs, const CNodeCandidate& rhs) {
        return lhs.weight > rhs.weight;
    }

    struct TracebackInfo {
        Mapping m;
        uint32_t parent;
        uint32_t mappingCost;
        uint32_t swapEstimatedCost;
    };

    struct MappingSwapSequence {
        std::vector<Mapping> mappings;
        std::vector<SwapSeq> swapSeqs;
        uint32_t cost;
    };
}
}

// --------------------- OptBMTQAllocator ------------------------
OptBMTQAllocator::OptBMTQAllocator(ArchGraph::sRef ag)
    : QbitAllocator(ag),
      mGen((std::random_device())()),
      mDistribution(0.0) {}

std::vector<MappingCandidate>
OptBMTQAllocator::extendCandidates(const Dep& dep,
                                   const std::vector<bool>& mapped,
                                   const std::vector<MappingCandidate>& candidates) {
    typedef std::pair<uint32_t, uint32_t> Pair;

    std::vector<MappingCandidate> newCandidates;
    uint32_t a = dep.mFrom, b = dep.mTo;

    for (auto cand : candidates) {
        std::vector<Pair> pairV;
        auto inv = InvertMapping(mPQubits, cand.m, false);

        if (mapped[a] && mapped[b]) {
            uint32_t u = cand.m[a], v = cand.m[b];

            if (mArchGraph->hasEdge(u, v) || mArchGraph->hasEdge(v, u)) {
                pairV.push_back(Pair(u, v));
            }

        } else if (!mapped[a] && !mapped[b]) {

            for (uint32_t u = 0; u < mPQubits; ++u) {
                if (inv[u] != _undef) continue;
                for (uint32_t v : mArchGraph->adj(u)) {
                    if (inv[v] != _undef) continue;
                    pairV.push_back(Pair(u, v));
                }
            }

        } else {
            uint32_t mappedV;

            if (!mapped[a]) mappedV = b;
            else mappedV = a;

            uint32_t u = cand.m[mappedV];

            for (uint32_t v : mArchGraph->adj(u)) {
                if (inv[v] == _undef) {
                    if (mappedV == a) pairV.push_back(Pair(u, v));
                    else pairV.push_back(Pair(v, u));
                }
            }
        }

        for (auto& pair : pairV) {
            auto cpy = cand;
            cpy.m[a] = pair.first;
            cpy.m[b] = pair.second;
            cpy.cost += getCXCost(pair.first, pair.second);
            newCandidates.push_back(cpy);
        }
    }

    return newCandidates;
}

void OptBMTQAllocator::setCandidatesWeight(std::vector<MappingCandidate>& candidates,
                                           Graph& lastPartitionGraph) {
    for (auto& candidate : candidates) {
        candidate.weight = 0;
    }

    for (uint32_t a = 0; a < mVQubits; ++a) {
        if (candidates[0].m[a] == _undef) continue;

        for (uint32_t b : lastPartitionGraph.succ(a)) {
            if (candidates[0].m[b] == _undef) continue;

            for (auto& candidate : candidates) {
                candidate.weight += mDistance[candidate.m[a]][candidate.m[b]];
            }
        }
    }
}

std::vector<MappingCandidate>
OptBMTQAllocator::filterCandidates(const std::vector<MappingCandidate>& candidates) {
    uint32_t selectionNumber = std::min(mMaxPartial, (uint32_t) candidates.size());

    if (selectionNumber >= (uint32_t) candidates.size())
        return candidates;

    INF << "Filtering " << candidates.size() << " candidates." << std::endl;

    std::vector<MappingCandidate> selected;

    std::priority_queue<MappingCandidate,
                        std::vector<MappingCandidate>,
                        std::greater<MappingCandidate>> queue;
    for (auto candidate : candidates) {
        queue.push(candidate);
    }

    for (uint32_t i = 0; i < selectionNumber; ++i) {
        selected.push_back(queue.top());
        queue.pop();
    }

    return selected;
}

std::vector<std::vector<MappingCandidate>> OptBMTQAllocator::phase1(QModule::Ref qmod) {
    // First Phase:
    //     in this phase, we divide the program in layers, such that each layer is satisfied
    //     by any of the mappings inside 'candidates'.
    //
    mPP.push_back(std::vector<Node::Ref>());

    std::vector<std::vector<MappingCandidate>> collection;
    std::vector<MappingCandidate> candidates { { Mapping(mVQubits, _undef), 0 } };
    std::vector<bool> mapped(mVQubits, false);

    Graph lastPartitionGraph(mVQubits);
    Graph partitionGraph(mVQubits);

    INF << "PHASE 1 >>>> Solving SIP Instances" << std::endl;

    // Building CircuitGraph and its iterator.
    auto cGraph = PassCache::Get<CircuitGraphBuilderPass>(qmod)->getData();
    auto it = cGraph.build_iterator();
    auto mXbitSize = cGraph.size();

    // Map for keeping track of the reached nodes.
    // It is mapped to an integer representing the Xbits that reached that
    // node. We won't pick it as a candidate until every node reach it.
    std::unordered_map<Node::Ref, uint32_t> reached;

    // Initializing the iterator to the next node (since the first is
    // a dummy input node).
    for (uint32_t i = 0; i < mXbitSize; ++i) {
        it.next(i);
        ++reached[it.get(i)];
    }

    bool finished = false;

    while (!finished) {
        bool changed;

        // We issue every single-qubit gate, since we don't really care
        // about it. It won't influence on the allocation.
        do {
            changed = false;

            for (uint32_t i = 0; i < mXbitSize; ++i) {
                if (it[i]->isGateNode() && it[i]->numberOfXbits() == 1) {
                    mPP.back().push_back(it.get(i));
                    it.next(i);
                    ++reached[it.get(i)];

                    changed = true;
                }
            }
        } while (changed);

        bool redo = false;

        // It sounds dumb to have this kind of thing, but we do it so that
        // we can ensure deterministic behaviour. Otherwise, we will be susceptible
        // to different results if we change the the order of CircuitNodes.
        std::set<CircuitNode::Ref> circuitNodeCandidatesSet;
        std::vector<CircuitNode::Ref> circuitNodeCandidatesVector;
        std::set<CircuitNode::Ref> toBeIssued;

        for (uint32_t i = 0; i < mXbitSize; ++i) {
            if (it[i]->isGateNode() && reached[it.get(i)] == it[i]->numberOfXbits()) {
                if (mDBuilder.getDeps(it.get(i)).empty()) {
                    toBeIssued.insert(it[i].get());
                } else {
                    auto node = it[i].get();
                    if (circuitNodeCandidatesSet.find(node) == circuitNodeCandidatesSet.end()) {
                        circuitNodeCandidatesSet.insert(it[i].get());
                        circuitNodeCandidatesVector.push_back(it[i].get());
                    }
                }
            }
        }

        for (auto& cnode : toBeIssued) {
            mPP.back().push_back(cnode->node());

            for (auto& i : cnode->getXbitsId()) {
                it.next(i);
                ++reached[it.get(i)];
            }

            redo = true;
        }

        // Whenever we reach something like a `barrier` or a `measure` statement,
        // where we don't have any dependencies, we should redo everything so that
        // we gather the largest number of node candidates as possible.
        if (redo) continue;

        // If there is no node candidate, it means that we have finished processing
        // all nodes in the circuit.
        if (circuitNodeCandidatesVector.empty()) break;

        std::priority_queue<CNodeCandidate,
                            std::vector<CNodeCandidate>,
                            std::greater<CNodeCandidate>> nodeQueue;

        for (auto cnode : circuitNodeCandidatesVector) {
            CNodeCandidate cNCand;

            cNCand.cNode = cnode;
            cNCand.dep = mDBuilder.getDeps(cnode->node())[0];

            uint32_t a = cNCand.dep.mFrom, b = cNCand.dep.mTo;

            // If we have the edge (a, b), so either:
            //     - `a` and `b` are close to each other in this partition; or
            //     - `a` and `b` were close to each other in the previous partition,
            //     and now up to one of them is mapped.
            //
            // Even though `partitionGraph` is an undirected graph, when we clear the
            // successors and predecessors later, we can't really be sure that
            // (a, b) => (b, a).
            if (partitionGraph.hasEdge(a, b) || partitionGraph.hasEdge(b, a)) {
                // We don't need to do nothing if we find this case, since we know
                // that both are mapped and close to each other.
                if (mapped[a] && mapped[b]) cNCand.weight = 1;
                else if (mapped[a] || mapped[b]) cNCand.weight = 2;
                else cNCand.weight = 3;
            } else {
                // The order here is a bit different, since we want to delay the creation
                // of a new partition as much as we can.
                if (mapped[a] && mapped[b]) cNCand.weight = 6;
                else if (mapped[a] || mapped[b]) cNCand.weight = 4;
                else cNCand.weight = 5;
            }

            nodeQueue.push(cNCand);
        }

        CNodeCandidate cNCand;
        std::vector<MappingCandidate> newCandidates;

        // In the order stablished above, the first dependency we get to satisfy,
        // we take it in, until there is no more nodes in the queue or until the
        // `newCandidates` generated is not empty.
        while (!nodeQueue.empty()) {
            cNCand = nodeQueue.top();
            nodeQueue.pop();

            newCandidates = extendCandidates(cNCand.dep, mapped, candidates);

            if (!newCandidates.empty()) {
                setCandidatesWeight(newCandidates, lastPartitionGraph);
                newCandidates = filterCandidates(newCandidates);
                break;
            }
        }

        if (newCandidates.empty()) {
            collection.push_back(candidates);

            // Reseting all data from the last partition.
            candidates = { { Mapping(mVQubits, _undef), 0 } };
            mapped.assign(mVQubits, false);
            mPP.push_back(std::vector<Node::Ref>());

            lastPartitionGraph = partitionGraph;
        } else {
            auto dep = cNCand.dep;
            uint32_t a = dep.mFrom, b = dep.mTo;

            if (!mapped[b]) {
                partitionGraph.succ(b).clear();
                partitionGraph.pred(b).clear();
                mapped[b] = true;
            }

            if (!mapped[a]) {
                partitionGraph.succ(a).clear();
                partitionGraph.pred(a).clear();
                mapped[a] = true;
            }

            mapped[a] = true;
            mapped[b] = true;
            partitionGraph.putEdge(a, b);

            candidates = newCandidates;
            mPP.back().push_back(cNCand.cNode->node());

            for (auto i : cNCand.cNode->getXbitsId()) {
                it.next(i);
                ++reached[it.get(i)];
            }
        }
    }

    collection.push_back(candidates);

    return collection;
}

uint32_t OptBMTQAllocator::getNearest(uint32_t u, const InverseMap& inv) {
    uint32_t minV = 0;
    uint32_t minDist = _undef;

    for (uint32_t v = 0; v < mPQubits; ++v) {
        if (inv[v] == _undef && mDistance[u][v] < minDist) {
            minDist = mDistance[u][v];
            minV = v;
        }
    }

    return minV;
}

void OptBMTQAllocator::propagateLiveQubits(const Mapping& fromM, Mapping& toM) {
    auto toInv = InvertMapping(mPQubits, toM, false);

    for (uint32_t i = 0; i < mVQubits; ++i) {
        if (toM[i] == _undef && fromM[i] != _undef) {
            if (toInv[fromM[i]] == _undef) {
                toM[i] = fromM[i];
            } else {
                toM[i] = getNearest(fromM[i], toInv);
            }

            toInv[toM[i]] = i;
        }
    }
}

uint32_t OptBMTQAllocator::estimateSwapCost(const Mapping& fromM, const Mapping& toM) {
    uint32_t totalDistance = 0;

    for (uint32_t i = 0, e = fromM.size(); i < e; ++i) {
        if (fromM[i] != _undef && toM[i] != _undef) {
            totalDistance += mDistance[fromM[i]][toM[i]];
        }
    }

    return totalDistance * 30;
}

std::vector<Mapping>
OptBMTQAllocator::tracebackPath(const std::vector<std::vector<TracebackInfo>>& mem,
                                uint32_t idx) {
    std::vector<Mapping> mappings;
    uint32_t layers = mem.size();

    for (int32_t i = layers - 1; i >= 0; --i) {
        auto info = mem[i][idx];
        mappings.push_back(info.m);
        idx = info.parent;
    }
    
    std::reverse(mappings.begin(), mappings.end());
    return mappings;
}

SwapSeq OptBMTQAllocator::getTransformingSwapsFor(const Mapping& fromM,
                                                  Mapping toM) {

    for (uint32_t i = 0; i < mVQubits; ++i) {
        EfdAbortIf(fromM[i] != _undef && toM[i] == _undef,
                   "Assumption that previous mappings have same mapped qubits "
                   << "than current mapping broken.");

        if (fromM[i] == _undef && toM[i] != _undef) {
            toM[i] = _undef;
        }
    }

    auto fromInv = InvertMapping(mPQubits, fromM, false);
    auto toInv = InvertMapping(mPQubits, toM, false);

    return mTSFinder->find(fromInv, toInv);
}

void OptBMTQAllocator::normalize(MappingSwapSequence& mss) {
    // Fill the last mapping, so that all qubits are mapped in the end.
    auto &lastMapping = mss.mappings.back();
    Fill(mPQubits, lastMapping);

    auto inv = InvertMapping(mPQubits, lastMapping);

    for (uint32_t i = mss.mappings.size() - 1; i > 0; --i) {
        mss.mappings[i - 1] = mss.mappings[i];
        Mapping& mapping = mss.mappings[i - 1];

        auto swaps = mss.swapSeqs[i - 1];

        for (auto it = swaps.rbegin(), end = swaps.rend(); it != end; ++it) {
            uint32_t u = it->u, v = it->v;
            uint32_t a = inv[u], b = inv[v];
            if (a < mVQubits) mapping[a] = v;
            if (b < mVQubits) mapping[b] = u;
            std::swap(inv[u], inv[v]);
        }
    }
}

MappingSwapSequence
OptBMTQAllocator::phase2(const std::vector<std::vector<MappingCandidate>>& collection) {
    // Second Phase:
    //     here, the idea is to use, perhaps, dynamic programming to test all possibilities
    //     for 'glueing' the sequence of collection together.
    uint32_t layers = collection.size();
    uint32_t layerMaxSize = 0;

    for (uint32_t i = 0; i < layers; ++i) {
        layerMaxSize = std::max(layerMaxSize, (uint32_t) collection[i].size());
    }
    
    INF << "PHASE 2 >>>> Dynamic Programming" << std::endl;
    INF << "Layers: " << layers << std::endl;
    INF << "MaxSize: " << layerMaxSize << std::endl;

    std::vector<std::vector<TracebackInfo>> mem(layers, std::vector<TracebackInfo>());

    for (uint32_t i = 0, e = collection[0].size(); i < e; ++i) {
        mem[0].push_back({ collection[0][i].m, _undef, collection[0][i].cost, 0 });
    }

    for (uint32_t i = 1; i < layers; ++i) {
        // INF << "Beginning: " << i << " of " << layers << " layers." << std::endl;

        uint32_t jLayerSize = collection[i].size();
        for (uint32_t j = 0; j < jLayerSize; ++j) {
            // Timer jt;
            // jt.start();

            TracebackInfo best = { {}, _undef, _undef, 0 };
            uint32_t kLayerSize = collection[i - 1].size();

            for (uint32_t k = 0; k < kLayerSize; ++k) {
                auto mapping = collection[i][j].m;

                propagateLiveQubits(mem[i - 1][k].m, mapping);

                uint32_t mappingCost = mem[i - 1][k].mappingCost + collection[i][j].cost;
                uint32_t swapEstimatedCost = estimateSwapCost(mem[i - 1][k].m, mapping) +
                                             mem[i - 1][k].swapEstimatedCost;
                uint32_t estimatedCost = mappingCost + swapEstimatedCost;

                if (estimatedCost < best.mappingCost + best.swapEstimatedCost) {
                    best = { mapping, k, mappingCost, swapEstimatedCost };
                }
            }

            mem[i].push_back(best);

            // jt.stop();
            // INF << "(i:" << i << ", j:" << j << "): "
            //     << ((double) jt.getMilliseconds()) / 1000.0 << std::endl;
        }

        INF << "End: " << i + 1 << " of " << layers << " layers." << std::endl;
    }

    MappingSwapSequence best = { {}, {}, _undef };

    for (uint32_t idx = 0, end = mem.back().size(); idx < end; ++idx) {
        std::vector<SwapSeq> swapSeqs;
        std::vector<Mapping> mappings = tracebackPath(mem, idx);

        uint32_t swapCost = 0;
        uint32_t mappingCost = mem.back()[idx].mappingCost;

        for (uint32_t i = 1; i < layers; ++i) {
            auto swaps = getTransformingSwapsFor(mappings[i - 1], mappings[i]);
            swapSeqs.push_back(swaps);

            for (const auto& s : swaps) {
                swapCost += getSwapCost(s.u, s.v);
            }
        }

        if (swapCost + mappingCost < best.cost) {
            best.mappings = mappings;
            best.swapSeqs = swapSeqs;
            best.cost = swapCost + mappingCost;
        }
    }

    normalize(best);
    return best;
}


Mapping OptBMTQAllocator::phase3(QModule::Ref qmod, const MappingSwapSequence& mss) {
    // Third Phase:
    //     build the operations vector by tracebacking from the solution we have
    //     found. For this, we have to go through every dependency again.
    uint32_t idx = 0;
    auto initial = mss.mappings[idx];
    auto mapping = initial;

    QubitRemapVisitor visitor(mapping, mXtoN);
    std::vector<Node::uRef> issuedInstructions;

    for (auto& partition : mPP) {
        if (idx > 0) {
            auto swaps = mss.swapSeqs[idx - 1];

            for (auto swp : swaps) {
                uint32_t u = swp.u, v = swp.v;

                if (!mArchGraph->hasEdge(u, v)) {
                    std::swap(u, v);
                }

                issuedInstructions.push_back(
                        CreateISwap(mArchGraph->getNode(u)->clone(),
                                    mArchGraph->getNode(v)->clone()));
            }
        }

        mapping = mss.mappings[idx++];

        for (auto& node : partition) {
            // We are sure that there are no instruction dependency that has more than
            // one dependency.
            auto iDependencies = mDBuilder.getDeps(node);

            if (iDependencies.size() < 1) {
                auto cloned = node->clone();
                cloned->apply(&visitor);
                issuedInstructions.push_back(std::move(cloned));
                continue;
            }

            auto dep = iDependencies[0];

            uint32_t a = dep.mFrom, b = dep.mTo;
            uint32_t u = mapping[a], v = mapping[b];

            EfdAbortIf((u == _undef || v == _undef) ||
                       (!mArchGraph->hasEdge(u, v) && !mArchGraph->hasEdge(v, u)),
                       "Can't satisfy dependency (" << u << ", " << v << ") "
                       << "with " << idx << "-th mapping: " << MappingToString(mapping));

            Node::uRef newNode;
            if (mArchGraph->hasEdge(u, v)) {
                newNode = node->clone();
                newNode->apply(&visitor);
            } else if (mArchGraph->hasEdge(v, u)) {
                newNode = CreateIRevCX(mArchGraph->getNode(u)->clone(),
                                       mArchGraph->getNode(v)->clone());
            } else {
                EfdAbortIf(true,
                           "Mapping " << MappingToString(mapping)
                           << " not able to satisfy dependency "
                           << "(" << a << "{" << u << "}, " << b << "{" << v << "})");
            }

            issuedInstructions.push_back(std::move(newNode));
        }
    }

    qmod->clearStatements();
    qmod->insertStatementLast(std::move(issuedInstructions));

    return initial;
}

void OptBMTQAllocator::init(QModule::Ref qmod) {
    mMaxPartial = MaxPartialSolutions.getVal();

    mDBuilder = PassCache::Get<DependencyBuilderWrapperPass>(qmod)->getData();
    mXtoN = PassCache::Get<XbitToNumberWrapperPass>(qmod)->getData();

    mTSFinder = SimplifiedApproxTSFinder::Create();
    mTSFinder->setGraph(mArchGraph.get());

    mBFSDistance.init(mArchGraph.get());
    mDistance.assign(mPQubits, std::vector<uint32_t>(mPQubits, 0));

    for (uint32_t i = 0; i < mPQubits; ++i) {
        for (uint32_t j = i + 1; j < mPQubits; ++j) {
            auto dist = mBFSDistance.get(i, j);
            mDistance[i][j] = dist;
            mDistance[j][i] = dist;
        }
    }
}

Mapping OptBMTQAllocator::allocate(QModule::Ref qmod) {
    init(qmod);

    uint32_t nofDeps = mDBuilder.getDependencies().size();
    auto initialMapping = IdentityMapping(mPQubits);

    if (nofDeps > 0) {
        Timer tPhase1, tPhase2, tPhase3;

        tPhase1.start();
        auto phase1Output = phase1(qmod);
        tPhase1.stop();

        tPhase2.start();
        auto phase2Output = phase2(phase1Output);
        tPhase2.stop();

        tPhase3.start();
        initialMapping = phase3(qmod, phase2Output);
        tPhase3.stop();

        // Stats collection.
        Phase1Time = (double) tPhase1.getMilliseconds() / 1000.0;
        Phase2Time = (double) tPhase2.getMilliseconds() / 1000.0;
        Phase3Time = (double) tPhase3.getMilliseconds() / 1000.0;
        Partitions = mPP.size();
    }

    return initialMapping;
}

OptBMTQAllocator::uRef OptBMTQAllocator::Create(ArchGraph::sRef ag) {
    return uRef(new OptBMTQAllocator(ag));
}
