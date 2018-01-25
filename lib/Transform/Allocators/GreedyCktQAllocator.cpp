#include "enfield/Transform/Allocators/GreedyCktQAllocator.h"
#include "enfield/Transform/Allocators/WeightedSIMappingFinder.h"
#include "enfield/Transform/CircuitGraphBuilderPass.h"
#include "enfield/Transform/PassCache.h"
#include "enfield/Support/BFSPathFinder.h"
#include "enfield/Support/Defs.h"

#include <algorithm>

using namespace efd;

enum PropKind { K_SWP, K_FRZ } type;

struct AllocProps {
    CircuitNode* cnode;
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

GreedyCktQAllocator::GreedyCktQAllocator(ArchGraph::sRef ag) : QbitAllocator(ag) {}

Solution GreedyCktQAllocator::executeAllocation(QModule::Ref qmod) {
    auto depPass = PassCache::Get<DependencyBuilderWrapperPass>(mMod);
    auto depBuilder = depPass->getData();
    auto& depsSet = depBuilder.getDependencies();

    auto cgbpass = PassCache::Get<CircuitGraphBuilderPass>(qmod);
    auto cgraph = cgbpass->getData();
    auto xbits = cgraph.size();

    BFSPathFinder bfs;

    auto mapfinder = WeightedSIMappingFinder::Create();
    auto mapping = mapfinder->find(mArchGraph.get(), depsSet);
    auto assign = GenAssignment(mArchGraph->size(), mapping);

    Solution sol { mapping, Solution::OpSequences(depsSet.size()), 0 };

    std::vector<Node::uRef> allocatedStatements;
    std::map<CircuitNode*, uint32_t> reached;
    std::vector<bool> marked(xbits, false);
    std::vector<bool> frozen(mapping.size(), false);

    uint32_t t = 0;

    while (allocatedStatements.size() < qmod->getNumberOfStmts()) {
        bool changed, redo = false;

        do {
            changed = false;

            for (uint32_t i = 0; i < xbits; ++i) {
                auto cnode = cgraph[i];

                if (cnode && cnode->qargsid.size() + cnode->cargsid.size() <= 1) {
                    allocatedStatements.push_back(cnode->node->clone());
                    cgraph[i] = cnode->child[i];
                    changed = true;
                }
            }

            redo = redo || changed;
        } while (changed);

        if (redo) continue;
        redo = false;

        // Reach gates with non-marked xbits and mark them.
        for (uint32_t i = 0; i < xbits; ++i) {
            auto cnode = cgraph[i];

            if (cnode && !marked[i]) {
                marked[i] = true;

                if (reached.find(cnode) == reached.end())
                    reached[cnode] = cnode->qargsid.size() + cnode->cargsid.size();
                --reached[cnode];
            }
        }

        std::set<CircuitNode*> allocatable;

        // Advance the xbits' cgraph and unmark them.
        for (uint32_t i = 0; i < xbits; ++i) {
            auto cnode = cgraph[i];

            if (cnode && !reached[cnode]) {
                allocatable.insert(cnode);
            }
        }

        assert(allocatable.size() >= 1 && "Every step has to be one allocatable node.");

        // Removing instructions that don't use only one qubit, but do not have any dependencies
        for (auto cnode : allocatable) {
            auto node = cnode->node;
            auto dep = depBuilder.getDeps(node);

            if (dep.getSize() == 0) {
                redo = true;
                allocatedStatements.push_back(node->clone());

                for (uint32_t q : cnode->qargsid) {
                    frozen[q] = true;
                    marked[q] = false;
                    cgraph[q] = cgraph[q]->child[q];
                }

                for (uint32_t c : cnode->cargsid) {
                    marked[c] = false;
                    cgraph[c] = cgraph[c]->child[c];
                }
            }
        }

        if (redo) continue;

        AllocProps best;
        best.cnode = nullptr;
        best.cost = _undef;

        for (auto cnode : allocatable) {
            // Calculate cost for allocating cnode->node;

            auto node = cnode->node;
            auto dep = depBuilder.getDeps(node);

            assert(dep.getSize() <= 1 && "Can only allocate gates with at most one depenency.");

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

                if (!frozen[a] || !frozen[b]) {
                    uint32_t notFrozen, theOther;

                    if (!frozen[b]) {
                        notFrozen = b; theOther = a;
                    } else {
                        notFrozen = a; theOther = b;
                    }

                    // This will repeat two times (if both 'a' and 'b' are not frozen)
                    while (!foundFrozen && !frozen[notFrozen]) {
                        uint32_t u = mapping[theOther];

                        for (uint32_t v : mArchGraph->adj(u)) {
                            uint32_t otherNotFrozen = assign[v];

                            if (!frozen[otherNotFrozen]) {
                                props.type = K_FRZ;
                                props.u.frz.from = notFrozen;
                                props.u.frz.to = otherNotFrozen;

                                if (!mArchGraph->hasEdge(u, mapping[otherNotFrozen]))
                                    props.cost = RevCost.getVal();

                                foundFrozen = true;
                                break;
                            }
                        }

                        std::swap(notFrozen, theOther);
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

        assert(best.cnode != nullptr && "There must be a 'best' node.");

        // Allocate best node;
        // Setting the 'stop' flag;
        auto& ops = sol.mOpSeqs[t++];
        auto node = best.cnode->node;
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
                    uint32_t a = assign[u], b = assign[v];

                    frozen[a] = true;
                    frozen[b] = true;

                    ops.second.push_back({ Operation::K_OP_SWAP, a, b });
                    std::cout << "SWAP(" << a << " => " << u << "; " << b << " => " << b << ")" << std::endl;

                    std::swap(mapping[a], mapping[b]);
                    std::swap(assign[u], assign[v]);
                }
            }
        } else {
            uint32_t a = best.u.frz.from, newA = best.u.frz.to;
            uint32_t u = mapping[a], v = mapping[newA];
            std::swap(sol.mInitial[a], sol.mInitial[newA]);
            std::swap(mapping[a], mapping[newA]);
            std::swap(assign[u], assign[v]);

            std::cout << "FRZ(" << a << " => " << mapping[a] << "; " << newA << " => " << mapping[newA] << ")" << std::endl;
        }

        // std::cout << "Cost: " << best.cost << std::endl;

        auto dep = depBuilder.getDeps(best.cnode->node)[0];
        uint32_t a = dep.mFrom, b = dep.mTo;

        frozen[a] = true;
        frozen[b] = true;

        if (mArchGraph->hasEdge(mapping[a], mapping[b])) {
            ops.second.push_back({ Operation::K_OP_CNOT, a, b });
            std::cout << "CNOT(" << a << " => " << mapping[a] << "; " << b << " => " << mapping[b] << ")" << std::endl;
        } else {
            ops.second.push_back({ Operation::K_OP_REV, a, b });
            std::cout << "REV(" << a << " => " << mapping[a] << "; " << b << " => " << mapping[b] << ")" << std::endl;
        }

        for (uint32_t q : best.cnode->qargsid) {
            marked[q] = false;
            cgraph[q] = cgraph[q]->child[q];
        }

        for (uint32_t c : best.cnode->cargsid) {
            marked[c] = true;
            cgraph[c] = cgraph[c]->child[c];
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
