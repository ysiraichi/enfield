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
    auto depPass = PassCache::Get<DependencyBuilderWrapperPass>(qmod);
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

    StdSolution sol { mapping, StdSolution::OpSequences(depsVector.size()) };

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

        EfdAbortIf(allocatable.empty(), "Every step has to process at least one gate.");

        // Removing instructions that don't use only one qubit, but do not have any dependencies
        for (auto cnode : allocatable) {
            auto node = cnode->node();
            auto dep = depBuilder.getDeps(node);

            if (dep.size() == 0) {
                redo = true;
                allocatedStatements.push_back(node->clone());

                for (uint32_t i : cnode->getXbitsId()) {
                    if (i < qubitNumber) frozen[i] = true;
                    marked[i] = false;
                    it.next(i);
                }
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

            EfdAbortIf(dep.size() > 1,
                       "Can only allocate gates with at most one depenency."
                       << " Gate: `" << dep.mCallPoint->toString(false) << "`.");

            uint32_t a = dep[0].mFrom, b = dep[0].mTo;
            uint32_t u = mapping[a], v = mapping[b];

            AllocProps props;
            props.cnode = cnode;
            props.cost = 0;
            props.path = {};

            if (mArchGraph->hasEdge(u, v) || mArchGraph->hasEdge(v, u)) {
                props.type = K_SWP;
                props.cost = getCXCost(u, v);
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
                            props.cost = getCXCost(u, v);
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
                            props.cost = getCXCost(u, v);
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
                    props.cost = 0;

                    if (mArchGraph->hasEdge(bfspath[0], bfspath[1])) {
                        props.u.swp.mvTgtSrc = true;

                        for (uint32_t i = pathsize - 1; i > 1; --i) {
                            props.cost += getSwapCost(bfspath[i], bfspath[i - 1]);
                        }

                        props.cost += getCXCost(bfspath[0], bfspath[1]);
                    } else if (mArchGraph->hasEdge(bfspath[pathsize - 2], bfspath[pathsize - 1])) {
                        props.u.swp.mvTgtSrc = false;

                        for (uint32_t i = 1; i < pathsize - 1; ++i) {
                            props.cost += getSwapCost(bfspath[i], bfspath[i - 1]);
                        }

                        props.cost += getCXCost(bfspath[pathsize - 2], bfspath[pathsize - 1]);
                    }

                }
            }

            if (best.cost > props.cost)
                best = props;
        }

        EfdAbortIf(best.cnode.get() == nullptr, "There must be a 'best' node.");

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

        for (uint32_t i : best.cnode->getXbitsId()) {
            marked[i] = false;
            it.next(i);
        }
    }

    qmod->clearStatements();
    qmod->insertStatementLast(std::move(allocatedStatements));

    return sol;
}

GreedyCktQAllocator::uRef GreedyCktQAllocator::Create(ArchGraph::sRef ag) {
    return uRef(new GreedyCktQAllocator(ag));
}
