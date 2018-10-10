#include "enfield/Transform/Allocators/BoundedMappingTreeQAllocator.h"
#include "enfield/Transform/Allocators/BMT/DefaultBMTQAllocatorImpl.h"
#include "enfield/Transform/QubitRemapPass.h"
#include "enfield/Transform/PassCache.h"
#include "enfield/Transform/Utils.h"
#include "enfield/Analysis/NodeVisitor.h"
#include "enfield/Support/ApproxTSFinder.h"
#include "enfield/Support/CommandLine.h"
#include "enfield/Support/Stats.h"
#include "enfield/Support/Defs.h"
#include "enfield/Support/Timer.h"

#include <algorithm>

using namespace efd;
using namespace bmt;

static Opt<uint32_t> MaxChildren
("-bmt-max-children", "Limits the max number of children per partial solution.",
 std::numeric_limits<uint32_t>::max(), false);

static Opt<uint32_t> MaxPartialSolutions
("-bmt-max-partial", "Limits the max number of partial solutions per step.",
 std::numeric_limits<uint32_t>::max(), false);

Stat<double> Phase1Time
("Phase1Time", "Time spent by the 1st phase of BMT allocators.");

Stat<double> Phase2Time
("Phase2Time", "Time spent by the 2nd phase of BMT allocators.");

Stat<double> Phase3Time
("Phase3Time", "Time spent by the 3rd phase of BMT allocators.");

Stat<uint32_t> Partitions
("BMTPartitions", "Number of partitions split by *BMT allocators.");

bool efd::bmt::operator>(const NodeCandidate& lhs, const NodeCandidate& rhs) {
    if (lhs.mWeight != rhs.mWeight) return lhs.mWeight > rhs.mWeight;
    return lhs.mNode > rhs.mNode;
}

// --------------------- NodeCandidatesGenerator ------------------------
NodeCandidatesGenerator::NodeCandidatesGenerator() : mInitialized(false), mMod(nullptr) {}

std::vector<Node::Ref> NodeCandidatesGenerator::generate() {
    checkInitialized();
    if (finished()) return {};
    return generateImpl();
}

bool NodeCandidatesGenerator::finished() {
    checkInitialized();
    return finishedImpl();
}

void NodeCandidatesGenerator::checkInitialized() {
    EfdAbortIf(!mInitialized, "`NodeCandidatesGenerator` not initialized.");
}

void NodeCandidatesGenerator::init(QModule::Ref qmod) {
    EfdAbortIf(qmod == nullptr, "Set the `QModule` for NodeCandidatesGenerator.");
    mInitialized = true;
    mMod = qmod;
    initImpl();
}

void NodeCandidatesGenerator::initImpl() {}
void NodeCandidatesGenerator::signalProcessed(Node::Ref node) {}

// --------------------- SwapCostEstimator ------------------------
SwapCostEstimator::SwapCostEstimator() : mG(nullptr) {}

void SwapCostEstimator::init(Graph::Ref g) {
    mG = g;
    initImpl();
}

uint32_t SwapCostEstimator::estimate(const Mapping& fromM, const Mapping& toM) {
    checkInitialized();
    return estimateImpl(fromM, toM);
}

void SwapCostEstimator::checkInitialized() {
    EfdAbortIf(mG == nullptr, "Set the `Graph` for SwapCostEstimator.");
}

// --------------------- LiveQubitsPreProcessor ------------------------
LiveQubitsPreProcessor::LiveQubitsPreProcessor() : mG(nullptr) {}

void LiveQubitsPreProcessor::init(Graph::Ref g) {
    mG = g;
    initImpl();
}

void LiveQubitsPreProcessor::process(Mapping& fromM, Mapping& toM) {
    checkInitialized();
    return processImpl(fromM, toM);
}

void LiveQubitsPreProcessor::checkInitialized() {
    EfdAbortIf(mG == nullptr, "Set the `Graph` for LiveQubitsPreProcessor.");
}

// --------------------- BoundedMappingTreeQAllocator ------------------------
BoundedMappingTreeQAllocator::BoundedMappingTreeQAllocator(ArchGraph::sRef ag)
    : QbitAllocator(ag),
      mNCGenerator(nullptr),
      mChildrenCSelector(nullptr),
      mPartialSolutionCSelector(nullptr),
      mCostEstimator(nullptr),
      mLQPProcessor(nullptr),
      mMSSelector(nullptr),
      mTSFinder(nullptr) {}

MCandidateVector
BoundedMappingTreeQAllocator::extendCandidates(Dep& dep,
                                               const std::vector<bool>& mapped,
                                               const MCandidateVector& candidates,
                                               bool ignoreChildrenLimit) {
    typedef std::pair<uint32_t, uint32_t> Pair;
    typedef std::vector<Pair> PairVector;

    uint32_t a = dep.mFrom, b = dep.mTo;
    uint32_t childrenBound = (ignoreChildrenLimit) ? _undef : mMaxChildren;
    MCandidateVector newCandidates;

    for (auto cand : candidates) {
        PairVector pairV;
        MCandidateVector localCandidates;
        auto inv = InvertMapping(mPQubits, cand.m, false);

        if (mapped[a] && mapped[b]) {
            uint32_t u = cand.m[a], v = cand.m[b];
            if (mArchGraph->hasEdge(u, v) || mArchGraph->hasEdge(v, u))
                pairV.push_back(Pair(u, v));
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

            if (!mapped[a]) {
                mappedV = b;
            } else {
                mappedV = a;
            }

            uint32_t u = cand.m[mappedV];
            for (uint32_t v : mArchGraph->adj(u)) {
                if (inv[v] == _undef) {
                    if (mappedV == a) {
                        pairV.push_back(Pair(u, v));
                    } else {
                        pairV.push_back(Pair(v, u));
                    }
                }
            }
        }

        for (auto& pair : pairV) {
            auto cpy = cand;
            cpy.m[a] = pair.first;
            cpy.m[b] = pair.second;
            cpy.cost += getCXCost(pair.first, pair.second);
            localCandidates.push_back(cpy);
        }

        MCandidateVector selected = mChildrenCSelector->select(childrenBound, localCandidates);
        newCandidates.insert(newCandidates.end(), selected.begin(), selected.end());
    }

    return mPartialSolutionCSelector->select(mMaxPartial, newCandidates);
}

NCPQueue
BoundedMappingTreeQAllocator::rankCandidates(const std::vector<Node::Ref>& nodeCandidates,
                                             const std::vector<bool>& mapped,
                                             const std::vector<std::set<uint32_t>>& neighbors) {
    NCPQueue queue;

    for (auto node : nodeCandidates) {
        NodeCandidate nCand;

        nCand.mNode = node;
        nCand.mDeps = mDBuilder.getDeps(node);

        auto depsSize = nCand.mDeps.size();
        if (depsSize == 0) {
            nCand.mWeight = 0;
        } else if (depsSize == 1) {
            uint32_t a = nCand.mDeps[0].mFrom;
            uint32_t b = nCand.mDeps[0].mTo;

            if (mapped[a] && mapped[b] && neighbors[a].find(b) != neighbors[a].end()) {
                nCand.mWeight = 1;
            } else if (!mapped[a] && !mapped[b]) {
                nCand.mWeight = 3;
            } else if (!mapped[a] || !mapped[b]) {
                nCand.mWeight = 2;
            } else {
                nCand.mWeight = 4;
            }
        } else {
            EfdAbortIf(true,
                       "Instructions with more than one dependency not supported ("
                       << node->toString(false) << ")");
        }

        queue.push(nCand);
    }

    EfdAbortIf(queue.empty(), "`pQueue` empty.");
    return queue;
}

MCandidateVCollection BoundedMappingTreeQAllocator::phase1() {
    // First Phase:
    //     in this phase, we divide the program in layers, such that each layer is satisfied
    //     by any of the mappings inside 'candidates'.
    //
    mPP.push_back(PPartition());
    MCandidateVector candidates { { Mapping(mVQubits, _undef), 0 } };
    MCandidateVCollection collection;

    std::vector<bool> mapped(mVQubits, false);
    std::vector<std::set<uint32_t>> neighbors(mVQubits);

    INF << "PHASE 1 >>>> Solving SIP Instances" << std::endl;

    bool first = true;

    while (!mNCGenerator->finished()) {
        auto nodeCandidates = mNCGenerator->generate();
        auto pQueue = rankCandidates(nodeCandidates, mapped, neighbors);

        NodeCandidate nCand;
        MCandidateVector newCandidates;

        while (!pQueue.empty()) {
            nCand = pQueue.top();
            pQueue.pop();

            auto depsSize = nCand.mDeps.size();

            if (depsSize == 0) {
                newCandidates = candidates;
                break;
            } else if (depsSize == 1) {
                newCandidates = extendCandidates(nCand.mDeps[0],
                                                 mapped,
                                                 candidates,
                                                 first);
                first = false;
                if (!newCandidates.empty()) {
                    break;
                }
            }
        }

        if (newCandidates.empty()) {
            collection.push_back(candidates);
            // Reseting all data from the last partition.
            candidates = { { Mapping(mVQubits, _undef), 0 } };
            mapped.assign(mVQubits, false);
            mPP.push_back(PPartition());
            first = true;

        } else {
            if (!nCand.mDeps.empty()) {
                auto dep = nCand.mDeps[0];
                uint32_t a = dep.mFrom, b = dep.mTo;

                mapped[a] = true;
                mapped[b] = true;
                neighbors[a].insert(b);
                neighbors[b].insert(a);

                candidates = newCandidates;
            }

            mPP.back().push_back(nCand.mNode);
            mNCGenerator->signalProcessed(nCand.mNode);
        }
    }

    collection.push_back(candidates);

    return collection;
}

MappingSeq BoundedMappingTreeQAllocator::tracebackPath(const TIMatrix& mem, uint32_t idx) {
    MappingSeq mapSeq;
    uint32_t nofLayers = mem.size();

    mapSeq.mappingCost = mem[nofLayers - 1][idx].mappingCost;
    
    for (int32_t i = nofLayers - 1; i >= 0; --i) {
        auto info = mem[i][idx];
        mapSeq.mappingV.push_back(info.m);
        idx = info.parent;
    }
    
    std::reverse(mapSeq.mappingV.begin(), mapSeq.mappingV.end());
    return mapSeq;
}

SwapSeq BoundedMappingTreeQAllocator::getTransformingSwapsFor(const Mapping& fromM,
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

void BoundedMappingTreeQAllocator::normalize(MappingSwapSequence& mss) {
    uint32_t mappingVSize = mss.mappingV.size();

    // Fill the last mapping, so that all qubits are mapped in
    // the end.
    auto &lastMapping = mss.mappingV.back();
    Fill(mPQubits, lastMapping);

    auto inv = InvertMapping(mPQubits, lastMapping);

    for (uint32_t i = mappingVSize - 1; i > 0; --i) {
        mss.mappingV[i - 1] = mss.mappingV[i];
        Mapping& mapping = mss.mappingV[i - 1];
        auto swaps = mss.swapSeqCollection[i - 1];

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
BoundedMappingTreeQAllocator::phase2(const MCandidateVCollection& collection) {
    // Second Phase:
    //     here, the idea is to use, perhaps, dynamic programming to test all possibilities
    //     for 'glueing' the sequence of collection together.
    uint32_t nofLayers = collection.size();
    uint32_t layerMaxSize = 0;

    for (uint32_t i = 0; i < nofLayers; ++i)
        layerMaxSize = std::max(layerMaxSize, (uint32_t) collection[i].size());
    
    INF << "PHASE 2 >>>> Dynamic Programming" << std::endl;
    INF << "Layers: " << nofLayers << std::endl;
    INF << "MaxSize: " << layerMaxSize << std::endl;

    TIMatrix mem(nofLayers, TIVector());

    for (uint32_t i = 0, e = collection[0].size(); i < e; ++i) {
        mem[0].push_back({ collection[0][i].m, _undef, collection[0][i].cost, 0 });
    }

    for (uint32_t i = 1; i < nofLayers; ++i) {
        // INF << "Beginning: " << i << " of " << nofLayers << " layers." << std::endl;

        uint32_t jLayerSize = collection[i].size();
        for (uint32_t j = 0; j < jLayerSize; ++j) {
            // Timer jt;
            // jt.start();

            TracebackInfo best = { {}, _undef, _undef, 0 };
            uint32_t kLayerSize = collection[i - 1].size();

            for (uint32_t k = 0; k < kLayerSize; ++k) {
                auto mapping = collection[i][j].m;
                auto lastMapping = mem[i - 1][k].m;

                mLQPProcessor->process(lastMapping, mapping);

                // uint32_t mappingCost = mem[i - 1][k].mappingCost;
                // Should be this one.
                uint32_t mappingCost = mem[i - 1][k].mappingCost + collection[i][j].cost;
                uint32_t swapEstimatedCost = mCostEstimator->estimate(lastMapping, mapping) * 30 +
                    mem[i - 1][k].swapEstimatedCost;

                if (mappingCost + swapEstimatedCost < best.mappingCost + best.swapEstimatedCost) {
                    best = { mapping, k, mappingCost, swapEstimatedCost };
                }
            }

            mem[i].push_back(best);

            // jt.stop();
            // INF << "(i:" << i << ", j:" << j << "): "
            //     << ((double) jt.getMilliseconds()) / 1000.0 << std::endl;
        }

        INF << "End: " << i << " of " << nofLayers << " layers." << std::endl;
    }

    Vector mapSequenceIndexes = mMSSelector->select(mem);
    MappingSwapSequence best = { {}, {}, _undef };

    for (uint32_t idx : mapSequenceIndexes) {
        // mapSCollection.push_back(tracebackPath(mem, idx));
        SwapSeqVector swapSeqCollection;
        auto seq = tracebackPath(mem, idx);

        uint32_t swapCost = 0;
        uint32_t mappingCost = seq.mappingCost;

        for (uint32_t i = 1; i < nofLayers; ++i) {
            auto swaps = getTransformingSwapsFor(seq.mappingV[i - 1], seq.mappingV[i]);
            swapSeqCollection.push_back(swaps);

            for (const auto& s : swaps) {
                swapCost += getSwapCost(s.u, s.v);
            }
        }

        if (swapCost + mappingCost < best.cost) {
            best.mappingV = seq.mappingV;
            best.swapSeqCollection = swapSeqCollection;
            best.cost = swapCost + mappingCost;
        }
    }

    normalize(best);
    return best;
}


Mapping BoundedMappingTreeQAllocator::phase3(QModule::Ref qmod, const MappingSwapSequence& mss) {
    // Third Phase:
    //     build the operations vector by tracebacking from the solution we have
    //     found. For this, we have to go through every dependency again.
    uint32_t idx = 0;
    auto initial = mss.mappingV[idx];
    auto mapping = initial;

    QubitRemapVisitor visitor(mapping, mXtoN);
    std::vector<Node::uRef> issuedInstructions;

    for (auto& partition : mPP) {
        for (auto& node : partition) {
        // for (auto& iDependencies : deps) {
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

            // If we can't satisfy (u, v) with the current mapping, it can only mean
            // that we must go to the next one.
            if ((u == _undef || v == _undef) ||
                (!mArchGraph->hasEdge(u, v) && !mArchGraph->hasEdge(v, u))) {

                EfdAbortIf(++idx >= mss.mappingV.size(),
                           "Not enough mappings were generated, maybe!? "
                           << "Mapping for '" << iDependencies.mCallPoint->toString(false) << "'.");

                mapping = mss.mappingV[idx];
                auto swaps = mss.swapSeqCollection[idx - 1];

                for (auto swp : swaps) {
                    uint32_t u = swp.u, v = swp.v;
                    if (!mArchGraph->hasEdge(u, v)) {
                        std::swap(u, v);
                    }
                    issuedInstructions.push_back(CreateISwap(mArchGraph->getNode(u)->clone(),
                                                             mArchGraph->getNode(v)->clone()));
                }

                u = mapping[a];
                v = mapping[b];
            }

            Node::uRef newNode;

            if (mArchGraph->hasEdge(u, v)) {
                newNode = node->clone();
                newNode->apply(&visitor);
            } else if (mArchGraph->hasEdge(v, u)) {
                newNode = CreateIRevCX(mArchGraph->getNode(u)->clone(),
                                       mArchGraph->getNode(v)->clone());
            } else {
                EfdAbortIf(true,
                           "Mapping " << MappingToString(mapping) <<
                           " not able to satisfy dependency ("
                           << a << "{" << u << "}, "
                           << b << "{" << v << "})");
            }

            issuedInstructions.push_back(std::move(newNode));
        }
    }

    qmod->clearStatements();
    qmod->insertStatementLast(std::move(issuedInstructions));

    return initial;
}

Mapping BoundedMappingTreeQAllocator::allocate(QModule::Ref qmod) {
    EfdAbortIf(mNCGenerator.get() == nullptr                ||
               mChildrenCSelector.get() == nullptr          ||
               mPartialSolutionCSelector.get() == nullptr   ||
               mCostEstimator.get() == nullptr              ||
               mLQPProcessor.get() == nullptr               ||
               mMSSelector.get() == nullptr                 ||
               mTSFinder.get() == nullptr,
               "Define the `BoundedMappingTreeQAllocator` interfaces:"
               << std::endl
               << "    NodeCandidatesGenerator: " << mNCGenerator.get()
               << std::endl
               << "    mChildrenCSelector: " << mChildrenCSelector.get()
               << std::endl
               << "    mPartialSolutionCSelector: " << mPartialSolutionCSelector.get()
               << std::endl
               << "    mCostEstimator: " << mCostEstimator.get()
               << std::endl
               << "    mLQPProcessor: " << mLQPProcessor.get()
               << std::endl
               << "    mMSSelector: " << mMSSelector.get()
               << std::endl
               << "    mTSFinder: " << mTSFinder.get());

    mNCGenerator->init(qmod);
    mLQPProcessor->init(mArchGraph.get());
    mCostEstimator->init(mArchGraph.get());

    mTSFinder->setGraph(mArchGraph.get());

    mMaxChildren = MaxChildren.getVal();
    mMaxPartial = MaxPartialSolutions.getVal();

    mDBuilder = PassCache::Get<DependencyBuilderWrapperPass>(qmod)->getData();
    mXtoN = PassCache::Get<XbitToNumberWrapperPass>(qmod)->getData();

    uint32_t nofDeps = mDBuilder.getDependencies().size();
    auto initialMapping = IdentityMapping(mPQubits);

    if (nofDeps > 0) {
        Timer tPhase1, tPhase2, tPhase3;

        tPhase1.start();
        auto phase1Output = phase1();
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

void BoundedMappingTreeQAllocator::setNodeCandidatesGenerator
(NodeCandidatesGenerator::uRef gen) {
    mNCGenerator = std::move(gen);
}

void BoundedMappingTreeQAllocator::setChildrenSelector
(CandidateSelector::uRef sel) {
    mChildrenCSelector = std::move(sel);
}

void BoundedMappingTreeQAllocator::setPartialSolutionSelector
(CandidateSelector::uRef sel) {
    mPartialSolutionCSelector = std::move(sel);
}

void BoundedMappingTreeQAllocator::setSwapCostEstimator
(SwapCostEstimator::uRef est) {
    mCostEstimator = std::move(est);
}

void BoundedMappingTreeQAllocator::setLiveQubitsPreProcessor
(LiveQubitsPreProcessor::uRef proc) {
    mLQPProcessor = std::move(proc);
}

void BoundedMappingTreeQAllocator::setMapSeqSelector
(MapSeqSelector::uRef sel) {
    mMSSelector = std::move(sel);
}

void BoundedMappingTreeQAllocator::setTokenSwapFinder
(TokenSwapFinder::uRef finder) {
    mTSFinder = std::move(finder);
}

BoundedMappingTreeQAllocator::uRef
BoundedMappingTreeQAllocator::Create(ArchGraph::sRef ag) {
    return uRef(new BoundedMappingTreeQAllocator(ag));
}
