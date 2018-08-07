#include "enfield/Transform/Allocators/GreedyCktQAllocator.h"
#include "enfield/Transform/Allocators/Simple/WeightedSIMappingFinder.h"
#include "enfield/Transform/CircuitGraphBuilderPass.h"
#include "enfield/Transform/PassCache.h"
#include "enfield/Support/BFSPathFinder.h"
#include "enfield/Support/Defs.h"

#include <algorithm>

using namespace efd;

enum PropKind { K_SWP, K_FRZ } type;

struct AllocProps {
    CircuitGraph::CircuitNode::sRef cnode;
    uint32_t cost;
    std::vector<uint32_t> path;

    PropKind type;

    union {
        struct {
            bool mvTgtSrc;
        } swp;

        struct {
            uint32_t from, to;
        } frz;
    } u;
};

GreedyCktQAllocator::GreedyCktQAllocator(ArchGraph::sRef ag) : StdSolutionQAllocator(ag) {}

StdSolution GreedyCktQAllocator::buildStdSolution(QModule::Ref qmod) {
    auto depPass = PassCache::Get<DependencyBuilderWrapperPass>(mMod);
    auto depBuilder = depPass->getData();
    auto& depsVector = depBuilder.getDependencies();

    auto cgbpass = PassCache::Get<CircuitGraphBuilderPass>(qmod);
    auto cgraph = cgbpass->getData();
    auto it = cgraph.build_iterator();

    auto xbitNumber = cgraph.size();
    auto qubitNumber = cgraph.getQSize();

    BFSPathFinder bfs;

    auto mapfinder = WeightedSIMappingFinder::Create();
    auto mapping = mapfinder->find(mArchGraph.get(), depsVector);
    auto inv = InvertMapping(mArchGraph->size(), mapping);

    StdSolution sol { mapping, StdSolution::OpSequences(depsVector.size()), 0 };

    std::vector<Node::uRef> allocatedStatements;
    std::map<Node::Ref, uint32_t> reached;
    std::vector<bool> marked(xbitNumber, false);
    std::vector<bool> frozen(qubitNumber, false);

    uint32_t t = 0;

    for (uint32_t i = 0; i < xbitNumber; ++i) {
        it.next(i);
    }

    while (allocatedStatements.size() < qmod->getNumberOfStmts()) {
        bool changed, redo = false;

        do {
            changed = false;

            for (uint32_t i = 0; i < xbitNumber; ++i) {
                auto cnode = it[i];

                if (cnode->isGateNode() && cnode->numberOfXbits() <= 1) {
                    allocatedStatements.push_back(cnode->node()->clone());
                    it.next(i);
                    changed = true;
                }
            }

            redo = redo || changed;
        } while (changed);

        if (redo) continue;
        redo = false;

        // Reach gates with non-marked xbitNumber and mark them.
        for (uint32_t i = 0; i < xbitNumber; ++i) {
            auto cnode = it[i];
            auto node = cnode->node();

            if (cnode->isGateNode() && !marked[i]) {
                marked[i] = true;

                if (reached.find(node) == reached.end())
                    reached[node] = cnode->numberOfXbits();
                --reached[node];
            }
        }

        std::set<CircuitGraph::CircuitNode::sRef> allocatable;

        // Advance the xbitNumber' cgraph and unmark them.
        for (uint32_t i = 0; i < xbitNumber; ++i) {
            auto cnode = it[i];
            auto node = cnode->node();

            if (cnode->isGateNode() && !reached[node]) {
                allocatable.insert(cnode);
            }
        }

        if (allocatable.empty()) {
            ERR << "Every step has to process at least one gate." << std::endl;
            ExitWith(ExitCode::EXIT_unreachable);
        }

        // Removing instructions that don't use only one qubit, but do not have any dependencies
        for (auto cnode : allocatable) {
            auto node = cnode->node();
            auto dep = depBuilder.getDeps(node);

            if (dep.size() == 0) {
                redo = true;
                allocatedStatements.push_back(node->clone());

                // for (uint32_t q : cnode->qargsid) {
                for (uint32_t i : cnode->getXbitsIds()) {
                    if (i < qubitNumber) frozen[i] = true;
                    marked[i] = false;
                    it.next(i);
                }

                // for (uint32_t c : cnode->cargsid) {
                //     marked[c] = false;
                //     cgraph[c] = cgraph[c]->child[c];
                // }
            }
        }

        if (redo) continue;

        AllocProps best;
        best.cnode = nullptr;
        best.cost = _undef;

        for (auto cnode : allocatable) {
            // Calculate cost for allocating cnode->node;

            auto node = cnode->node();
            auto dep = depBuilder.getDeps(node);

            if (dep.size() > 1) {
                ERR << "Can only allocate gates with at most one depenency."
                    << " Gate: `" << dep.mCallPoint->toString(false) << "`." << std::endl;
                ExitWith(ExitCode::EXIT_unreachable);
            }

            uint32_t a = dep[0].mFrom, b = dep[0].mTo;
            uint32_t u = mapping[a], v = mapping[b];

            AllocProps props;
            props.cnode = cnode;
            props.cost = 0;
            props.path = {};

            // If either 'a' or 'b' is not frozen, we can search for a vertice nearby (each of them)
            // in order not to o any swaps.
            bool hasEdge = mArchGraph->hasEdge(u, v);
            bool hasReverseEdge = mArchGraph->hasEdge(v, u);

            if (hasEdge) {
                props.type = K_SWP;
            } else if (!hasEdge && hasReverseEdge) {
                props.type = K_SWP;
                props.cost = RevCost.getVal();
            } else {
                bool foundFrozen = false;

                if (!frozen[a]) {
                    uint32_t v = mapping[b];

                    for (uint32_t u : mArchGraph->adj(v)) {
                        uint32_t newA = inv[u];

                        if (!frozen[newA]) {
                            props.type = K_FRZ;
                            props.u.frz.from = a;
                            props.u.frz.to = newA;

                            if (!mArchGraph->hasEdge(u, v))
                                props.cost = RevCost.getVal();

                            foundFrozen = true;
                            break;
                        }
                    }
                }

                if (!foundFrozen && !frozen[b]) {
                    uint32_t u = mapping[a];

                    for (uint32_t v : mArchGraph->adj(u)) {
                        uint32_t newB = inv[v];

                        if (!frozen[newB]) {
                            props.type = K_FRZ;
                            props.u.frz.from = b;
                            props.u.frz.to = newB;

                            if (!mArchGraph->hasEdge(u, v))
                                props.cost = RevCost.getVal();

                            foundFrozen = true;
                            break;
                        }
                    }
                }

                if (!foundFrozen) {
                    props.type = K_SWP;

                    auto bfspath = bfs.find(mArchGraph.get(), u, v);
                    uint32_t pathsize = bfspath.size();

                    props.path = bfspath;
                    props.cost = (bfspath.size() - 2) * SwapCost.getVal();

                    bool hasEdgeFromU = mArchGraph->hasEdge(bfspath[0], bfspath[1]);
                    bool hasEdgeToV = mArchGraph->hasEdge(bfspath[pathsize - 2], bfspath[pathsize - 1]);
                    if (hasEdgeFromU)
                        props.u.swp.mvTgtSrc = true;
                    else if (!hasEdgeFromU && hasEdgeToV)
                        props.u.swp.mvTgtSrc = false;
                    else if (!hasEdgeFromU && !hasEdgeToV)
                        props.cost += RevCost.getVal();
                }
            }

            if (best.cost > props.cost)
                best = props;
        }

        if (best.cnode.get() == nullptr) {
            ERR << "There must be a 'best' node." << std::endl;
            ExitWith(ExitCode::EXIT_unreachable);
        }

        // Allocate best node;
        // Setting the 'stop' flag;
        auto& ops = sol.mOpSeqs[t++];
        auto node = best.cnode->node();
        auto newNode = node->clone();

        ops.first = newNode.get();
        allocatedStatements.push_back(std::move(newNode));

        if (best.type == K_SWP) {
            if (best.u.swp.mvTgtSrc)
                std::reverse(best.path.begin(), best.path.end());

            if (best.path.size() > 2) {
                for (auto it = best.path.begin() + 2, end = best.path.end();
                        it != end; ++it) {
                    uint32_t u = *(it - 2), v = *(it - 1);
                    uint32_t a = inv[u], b = inv[v];

                    frozen[a] = true;
                    frozen[b] = true;

                    ops.second.push_back({ Operation::K_OP_SWAP, a, b });

                    std::swap(mapping[a], mapping[b]);
                    std::swap(inv[u], inv[v]);
                }
            }
        } else {
            uint32_t a = best.u.frz.from, newA = best.u.frz.to;
            uint32_t u = mapping[a], v = mapping[newA];
            std::swap(sol.mInitial[a], sol.mInitial[newA]);
            std::swap(mapping[a], mapping[newA]);
            std::swap(inv[u], inv[v]);
        }

        auto dep = depBuilder.getDeps(node)[0];
        uint32_t a = dep.mFrom, b = dep.mTo;

        frozen[a] = true;
        frozen[b] = true;

        if (mArchGraph->hasEdge(mapping[a], mapping[b])) {
            ops.second.push_back({ Operation::K_OP_CNOT, a, b });
        } else {
            ops.second.push_back({ Operation::K_OP_REV, a, b });
        }

        for (uint32_t i : best.cnode->getXbitsIds()) {
            marked[i] = false;
            it.next(i);
        }

        sol.mCost += best.cost;
    }

    qmod->clearStatements();
    for (auto& node : allocatedStatements) {
        qmod->insertStatementLast(std::move(node));
    }

    return sol;
}

GreedyCktQAllocator::uRef GreedyCktQAllocator::Create(ArchGraph::sRef ag) {
    return uRef(new GreedyCktQAllocator(ag));
}
