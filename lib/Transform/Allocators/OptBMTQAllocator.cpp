#include "enfield/Transform/Allocators/OptBMTQAllocator.h"
#include "enfield/Transform/Allocators/BMT/DefaultBMTQAllocatorImpl.h"
#include "enfield/Transform/PassCache.h"
#include "enfield/Transform/CircuitGraphBuilderPass.h"
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

using CircuitNode = CircuitGraph::CircuitNode;

static Opt<uint32_t> MaxPartialSolutions
("-bmt-max-partial", "Limits the max number of partial solutions per step.",
 std::numeric_limits<uint32_t>::max(), false);

extern Stat<double> Phase1Time;
extern Stat<double> Phase2Time;
extern Stat<double> Phase3Time;
extern Stat<uint32_t> Partitions;

// --------------------- OptNodeRenameVisitor ------------------------
namespace efd {
    class OptNodeRenameVisitor : public NodeVisitor {
        private:
            Mapping& mMap;
            XbitToNumber& mXtoN;
            ArchGraph::Ref mArchGraph;

        public:
            OptNodeRenameVisitor(Mapping& m,
                              XbitToNumber& xtoN,
                              ArchGraph::Ref archGraph);

            void visitNDQOp(NDQOp::Ref qop);

            void visit(NDQOpMeasure::Ref ref) override;
            void visit(NDQOpReset::Ref ref) override;
            void visit(NDQOpU::Ref ref) override;
            void visit(NDQOpCX::Ref ref) override;
            void visit(NDQOpBarrier::Ref ref) override;
            void visit(NDQOpGen::Ref ref) override;
            void visit(NDIfStmt::Ref ref) override;
    };
}

OptNodeRenameVisitor::OptNodeRenameVisitor(Mapping& m,
                                     XbitToNumber& xtoN,
                                     ArchGraph::Ref archGraph)
    : mMap(m), mXtoN(xtoN), mArchGraph(archGraph) {}

void OptNodeRenameVisitor::visitNDQOp(NDQOp::Ref qop) {
    auto qargs = qop->getQArgs();
    NDList::uRef newQArgs = NDList::Create();

    for (auto& qarg : *qargs) {
        uint32_t pseudoQUId = mXtoN.getQUId(qarg->toString(false));
        uint32_t physicalQUId = mMap[pseudoQUId];
        newQArgs->addChild(mArchGraph->getNode(physicalQUId)->clone());
    }

    qop->setQArgs(std::move(newQArgs));
}

void OptNodeRenameVisitor::visit(NDQOpMeasure::Ref ref) {
    visitNDQOp((NDQOp::Ref) ref);
}

void OptNodeRenameVisitor::visit(NDQOpReset::Ref ref) {
    visitNDQOp((NDQOp::Ref) ref);
}

void OptNodeRenameVisitor::visit(NDQOpU::Ref ref) {
    visitNDQOp((NDQOp::Ref) ref);
}

void OptNodeRenameVisitor::visit(NDQOpCX::Ref ref) {
    visitNDQOp((NDQOp::Ref) ref);
}

void OptNodeRenameVisitor::visit(NDQOpBarrier::Ref ref) {
    visitNDQOp((NDQOp::Ref) ref);
}

void OptNodeRenameVisitor::visit(NDQOpGen::Ref ref) {
    visitNDQOp((NDQOp::Ref) ref);
}

void OptNodeRenameVisitor::visit(NDIfStmt::Ref ref) {
    ref->getQOp()->apply(this);
}

// --------------------- OptBMTQAllocator:POD ------------------------
namespace efd {
    struct MappingCandidate {
        Mapping m;
        uint32_t cost;
    };
    
    struct CNodeCandidate {
        Dep dep;
        CircuitNode::Ref cNode;
        uint32_t weight;
    };

    bool operator>(const CNodeCandidate& lhs, const CNodeCandidate& rhs) {
        if (lhs.weight != rhs.weight) return lhs.weight > rhs.weight;
        return lhs.cNode->node() > rhs.cNode->node();
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

std::vector<MappingCandidate>
OptBMTQAllocator::filterCandidates(const std::vector<MappingCandidate>& candidates) {
    uint32_t selectionNumber = std::min(mMaxPartial, (uint32_t) candidates.size());

    if (selectionNumber >= (uint32_t) candidates.size())
        return candidates;

    uint32_t sqSum = 0;
    uint32_t wSum = 0;

    std::vector<MappingCandidate> selected;
    std::vector<uint32_t> weight;
    std::vector<bool> wasSelected(candidates.size(), false);

    for (const auto& cand : candidates) {
        sqSum += (cand.cost * cand.cost);
    }

    if (sqSum == 0) {
        weight.assign(candidates.size(), 1);
    } else {
        for (const auto& cand : candidates) {
            weight.push_back(sqSum - (cand.cost * cand.cost));
        }
    }

    for (auto w : weight) {
        wSum += w;
    }

    for (uint32_t i = 0; i < selectionNumber; ++i) {
        double r = mDistribution(mGen);
        double cummulativeProbability = 0;
        uint32_t j = 0;

        while (cummulativeProbability < r && j < weight.size()) {
            if (!wasSelected[j]) cummulativeProbability += (double) weight[j] /
                                                           ((double) wSum);
            ++j;
        }

        --j;
        wSum -= weight[j];
        wasSelected[j] = true;
        selected.push_back(candidates[j]);
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
    std::vector<std::set<uint32_t>> neighbors(mVQubits);

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
        std::set<CircuitNode::Ref> nodeCandidateSet;

        for (uint32_t i = 0; i < mXbitSize; ++i) {
            if (it[i]->isGateNode() && reached[it.get(i)] == it[i]->numberOfXbits()) {
                if (mDBuilder.getDeps(it.get(i)).empty()) {
                    mPP.back().push_back(it.get(i));
                    it.next(i);
                    ++reached[it.get(i)];
                    redo = true;
                } else {
                    nodeCandidateSet.insert(it[i].get());
                }
            }
        }

        // Whenever we reach something like a `barrier` or a `measure` statement,
        // where we don't have any dependencies, we should redo everything so that
        // we gather the largest number of node candidates as possible.
        if (redo) continue;

        // If there is no node candidate, it means that we have finished processing
        // all nodes in the circuit.
        if (nodeCandidateSet.empty()) break;

        std::priority_queue<CNodeCandidate,
                            std::vector<CNodeCandidate>,
                            std::greater<CNodeCandidate>> nodeQueue;

        for (auto cnode : nodeCandidateSet) {
            CNodeCandidate cNCand;

            cNCand.cNode = cnode;
            cNCand.dep = mDBuilder.getDeps(cnode->node())[0];

            uint32_t a = cNCand.dep.mFrom, b = cNCand.dep.mTo;

            if (mapped[a] && mapped[b] && neighbors[a].find(b) != neighbors[a].end()) {
                cNCand.weight = 1;
            } else if (!mapped[a] && !mapped[b]) {
                cNCand.weight = 3;
            } else if (!mapped[a] || !mapped[b]) {
                cNCand.weight = 2;
            } else {
                cNCand.weight = 4;
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

        } else {
            auto dep = cNCand.dep;
            uint32_t a = dep.mFrom, b = dep.mTo;

            mapped[a] = true;
            mapped[b] = true;
            neighbors[a].insert(b);
            neighbors[b].insert(a);

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
    std::vector<bool> visited(mPQubits, false);
    std::queue<uint32_t> q;
    q.push(u);
    visited[u] = true;

    while (!q.empty()) {
        uint32_t v = q.front();
        q.pop();

        if (inv[v] == _undef) return v;

        for (uint32_t w : mArchGraph->succ(v)) {
            if (!visited[w]) {
                visited[w] = true;
                q.push(w);
            }
        }

        for (uint32_t w : mArchGraph->pred(v)) {
            if (!visited[w]) {
                visited[w] = true;
                q.push(w);
            }
        }
    }

    // There is no way we can not find anyone!!
    ERR << "Can't find any vertice connected to v:" << u << "." << std::endl;
    EFD_ABORT();
}

void OptBMTQAllocator::propagateLiveQubits(Mapping& fromM, Mapping& toM) {
    auto fromInv = InvertMapping(mPQubits, fromM, false);
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

std::vector<uint32_t> OptBMTQAllocator::distanceFrom(uint32_t u) {
    std::vector<uint32_t> distance(mPQubits, _undef);
    std::queue<uint32_t> q;
    std::vector<bool> visited(mPQubits, false);

    q.push(u);
    visited[u] = true;
    distance[u] = 0;

    while (!q.empty()) {
        uint32_t u = q.front();
        q.pop();

        for (uint32_t v : mArchGraph->adj(u)) {
            if (!visited[v]) {
                visited[v] = true;
                distance[v] = distance[u] + 1;
                q.push(v);
            }
        }
    }

    return distance;
}

uint32_t OptBMTQAllocator::estimateSwapCost(const Mapping& fromM, const Mapping& toM) {
    uint32_t totalDistance = 0;

    for (uint32_t i = 0, e = fromM.size(); i < e; ++i) {
        if (fromM[i] != _undef) {
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
        if (fromM[i] != _undef && toM[i] == _undef) {
            ERR << "Assumption that previous mappings have same mapped qubits "
                << "than current mapping broken." << std::endl;
            EFD_ABORT();
        } else if (fromM[i] == _undef && toM[i] != _undef) {
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
                auto lastMapping = mem[i - 1][k].m;

                propagateLiveQubits(lastMapping, mapping);

                // uint32_t mappingCost = mem[i - 1][k].mappingCost;
                // Should be this one.
                uint32_t mappingCost = mem[i - 1][k].mappingCost + collection[i][j].cost;
                uint32_t swapEstimatedCost = estimateSwapCost(lastMapping, mapping) +
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

        INF << "End: " << i << " of " << layers << " layers." << std::endl;
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

    OptNodeRenameVisitor renameVisitor(mapping, mXtoN, mArchGraph.get());
    std::vector<Node::uRef> issuedInstructions;

    for (auto& partition : mPP) {
        for (auto& node : partition) {
        // for (auto& iDependencies : deps) {
            // We are sure that there are no instruction dependency that has more than
            // one dependency.
            auto iDependencies = mDBuilder.getDeps(node);

            if (iDependencies.size() < 1) {
                auto cloned = node->clone();
                cloned->apply(&renameVisitor);
                issuedInstructions.push_back(std::move(cloned));
                continue;
            }

            auto dep = iDependencies[0];

            uint32_t a = dep.mFrom, b = dep.mTo;
            uint32_t u = mapping[a], v = mapping[b];

            // If we can't satisfy (u, v) with the current mapping, it can only mean
            // that we must go to the next one.
            if ((u == _undef || v == _undef) ||
                (!mArchGraph->hasEdge(u, v) && !mArchGraph->hasEdge(v, u))) {

                if (++idx >= mss.mappings.size()) {
                    ERR << "Not enough mappings were generated, maybe!?" << std::endl;
                    ERR << "Mapping for '" << iDependencies.mCallPoint->toString(false)
                        << "'." << std::endl;
                    EFD_ABORT();
                }

                mapping = mss.mappings[idx];
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

                u = mapping[a];
                v = mapping[b];
            }

            Node::uRef newNode;

            if (mArchGraph->hasEdge(u, v)) {
                newNode = node->clone();
                newNode->apply(&renameVisitor);
            } else if (mArchGraph->hasEdge(v, u)) {
                newNode = CreateIRevCX(mArchGraph->getNode(u)->clone(),
                                       mArchGraph->getNode(v)->clone());
            } else {
                ERR << "Mapping " << MappingToString(mapping) << " not able to satisfy dependency "
                    << "(" << a << "{" << u << "}, " << b << "{" << v << "})" << std::endl;
                EFD_ABORT();
            }

            issuedInstructions.push_back(std::move(newNode));
        }
    }

    qmod->clearStatements();
    for (auto& instr : issuedInstructions) {
        qmod->insertStatementLast(std::move(instr));
    }

    return initial;
}

void OptBMTQAllocator::init(QModule::Ref qmod) {
    mMaxPartial = MaxPartialSolutions.getVal();

    mDBuilder = PassCache::Get<DependencyBuilderWrapperPass>(qmod)->getData();
    mXtoN = PassCache::Get<XbitToNumberWrapperPass>(qmod)->getData();

    mTSFinder = SimplifiedApproxTSFinder::Create();
    mTSFinder->setGraph(mArchGraph.get());

    for (uint32_t i = 0; i < mPQubits; ++i) {
        mDistance.push_back(distanceFrom(i));
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
