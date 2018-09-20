#include "enfield/Transform/Allocators/SabreQAllocator.h"
#include "enfield/Transform/Allocators/Simple/RandomMappingFinder.h"
#include "enfield/Transform/CircuitGraphBuilderPass.h"
#include "enfield/Transform/QubitRemapPass.h"
#include "enfield/Transform/PassCache.h"
#include "enfield/Transform/Utils.h"
#include "enfield/Support/CommandLine.h"
#include "enfield/Support/Defs.h"

#include <numeric>

using namespace efd;

using CircuitNode = CircuitGraph::CircuitNode;
using WeightedSwap = std::pair<double, Swap>;

static Opt<uint32_t> LookAhead
("-sabre-lookahead", "Sets the number of instructions to peek.", 20, false);
static Opt<uint32_t> Iterations
("-sabre-iterations", "Sets the number of times to run SABRE.", 5, false);

namespace {
    struct WeightedSwapCompare {
        bool operator()(const WeightedSwap& lhs, const WeightedSwap& rhs) {
            return lhs.first < rhs.first;
        }
    };
}

SabreQAllocator::SabreQAllocator(ArchGraph::sRef ag)
    : QbitAllocator(ag) {}

SabreQAllocator::MappingAndNSwaps
SabreQAllocator::allocateWithInitialMapping(const Mapping& initialMapping,
                                            QModule::Ref qmod,
                                            bool issueInstructions) {
    auto mapping = initialMapping;
    auto stmtNumber = qmod->getNumberOfStmts();

    auto depBuilder = PassCache::Get<DependencyBuilderWrapperPass>(qmod)->getData();
    auto cGraph = PassCache::Get<CircuitGraphBuilderPass>(qmod)->getData();

    auto it = cGraph.build_iterator();
    auto xbitNumber = cGraph.size();

    std::map<Node::Ref, uint32_t> reached;
    std::set<Node::Ref> pastLookAhead;

    std::vector<Node::uRef> newStatements;
    QubitRemapVisitor visitor(mapping, mXbitToNumber);

    uint32_t swapNum = 0;

    for (uint32_t i = 0; i < xbitNumber; ++i) {
        it.next(i);
        ++reached[it.get(i)];
    }

    while (true) {
        bool changed;

        do {
            changed = false;
            std::set<CircuitNode::Ref> issueNodes;

            for (uint32_t i = 0; i < xbitNumber; ++i) {
                auto cNode = it[i];
                auto node = cNode->node();

                if (cNode->isGateNode() && cNode->numberOfXbits() == reached[node]) {
                    switch (node->getKind()) {
                        case Node::Kind::K_QOP_U:
                        case Node::Kind::K_QOP_CX:
                        case Node::Kind::K_QOP_GEN:
                            if (cNode->numberOfXbits() > 1) {
                                auto deps = depBuilder.getDeps(node);

                                if (deps.size() != 1) {
                                    ERR << "Unable to handle `" << deps.size() << "` dependencies "
                                        << "in: `" << node->toString(false) << "`."
                                        << std::endl;
                                    EFD_ABORT();
                                }

                                auto dep = deps[0];
                                uint32_t u = mapping[dep.mFrom], v = mapping[dep.mTo];

                                if (!mArchGraph->hasEdge(u, v) &&
                                    !mArchGraph->hasEdge(v, u)) {
                                    break;
                                }
                            }

                        case Node::Kind::K_QOP_BARRIER:
                        case Node::Kind::K_QOP_MEASURE:
                            issueNodes.insert(cNode.get());
                            changed = true;
                            break;

                        default:
                            break;
                    }
                }
            }

            for (auto cNode : issueNodes) {
                for (auto i : cNode->getXbitsId()) {
                    it.next(i);
                    ++reached[it.get(i)];
                }

                if (issueInstructions) {
                    auto clone = cNode->node()->clone();
                    clone->apply(&visitor);
                    newStatements.push_back(std::move(clone));
                }
            }
        } while (changed);

        std::vector<Dep> currentLayer;
        std::vector<Dep> nextLayer;

        uint32_t offset = std::numeric_limits<uint32_t>::max();

        for (uint32_t i = 0; i < xbitNumber; ++i) {
            if (it[i]->isGateNode() && it[i]->numberOfXbits() == reached[it.get(i)]) {
                auto node = it.get(i);
                auto dep = depBuilder.getDeps(node)[0];

                pastLookAhead.insert(node);
                currentLayer.push_back(dep);

                offset = std::min(offset, (uint32_t) std::distance(qmod->stmt_begin(),
                                                                   qmod->findStatement(node)));
            }
        }

        // If there is no node in the current layer, it means that
        // we have reached the end of the algorithm. i.e. we processed
        // all nodes already.
        if (currentLayer.empty()) break;

        for (uint32_t i = offset, maxI = offset + mLookAhead; i < stmtNumber && i < maxI; ++i) {
            auto node = qmod->getStatement(i);
            if (pastLookAhead.find(node) == pastLookAhead.end()) {
                nextLayer.push_back(depBuilder.getDeps(node)[0]);
            }
        }

        std::set<uint32_t> usedQubits;
        std::set<WeightedSwap, WeightedSwapCompare> swapCandidates;

        for (const auto& dep : currentLayer) {
            usedQubits.insert(mapping[dep.mFrom]);
            usedQubits.insert(mapping[dep.mTo]);
        }

        auto invM = InvertMapping(mPQubits, mapping);

        for (auto u : usedQubits) {
            for (auto v : mArchGraph->adj(u)) {
                auto cpy = mapping;
                std::swap(cpy[invM[u]], cpy[invM[v]]);

                double currentLCost = 0;
                double nextLCost = 0;

                for (const auto& dep : currentLayer) {
                    currentLCost += mBFSDistance.get(cpy[dep.mFrom], cpy[dep.mTo]);
                }

                for (const auto& dep : nextLayer) {
                    nextLCost += mBFSDistance.get(cpy[dep.mFrom], cpy[dep.mTo]);
                }

                double cost = currentLCost + 0.5 * nextLCost;
                swapCandidates.insert(WeightedSwap(cost, Swap { u, v }));
            }
        }

        auto swap = swapCandidates.begin()->second;
        std::swap(mapping[invM[swap.u]], mapping[invM[swap.v]]);

        if (issueInstructions) {
            newStatements.push_back(
                    CreateISwap(mArchGraph->getNode(swap.u)->clone(),
                                mArchGraph->getNode(swap.v)->clone()));
        }

        ++swapNum;
    }

    if (issueInstructions) {
        qmod->clearStatements();
        for (auto& stmt : newStatements) {
            qmod->insertStatementLast(std::move(stmt));
        }
    }

    return MappingAndNSwaps(mapping, swapNum);
}

Mapping SabreQAllocator::allocate(QModule::Ref qmod) {
    std::vector<uint32_t> order(qmod->getNumberOfStmts());
    std::iota(order.begin(), order.end(), 0);
    std::reverse(order.begin(), order.end());

    mLookAhead = LookAhead.getVal();
    mIterations = Iterations.getVal();

    auto depBuilder = PassCache::Get<DependencyBuilderWrapperPass>(qmod)->getData();
    mXbitToNumber = depBuilder.getXbitToNumber();

    auto qmodReverse = qmod->clone();
    qmodReverse->orderby(order);

    mBFSDistance.init(mArchGraph.get());

    Mapping initialM, finalM;

    RandomMappingFinder mappingFinder;
    auto dummyDependencies = depBuilder.getDependencies();
    initialM = mappingFinder.find(mArchGraph.get(), dummyDependencies);

    MappingAndNSwaps best(initialM, std::numeric_limits<uint32_t>::max());

    for (uint32_t i = 0; i < mIterations; ++i) {
        auto resultFinal = allocateWithInitialMapping(initialM, qmod, false);
        auto resultInit = allocateWithInitialMapping(resultFinal.first, qmodReverse.get(), false);
        resultFinal = allocateWithInitialMapping(resultInit.first, qmod, false);

        if (resultFinal.second < best.second) {
            best = MappingAndNSwaps(resultInit.first, resultFinal.second);
        }
    }

    allocateWithInitialMapping(best.first, qmod, true);

    return best.first;
}

SabreQAllocator::uRef SabreQAllocator::Create(ArchGraph::sRef ag) {
    return uRef(new SabreQAllocator(ag));
}
