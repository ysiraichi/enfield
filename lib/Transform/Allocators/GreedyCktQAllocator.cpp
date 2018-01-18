#include "enfield/Transform/Allocators/GreedyCktQAllocator.h"
#include "enfield/Transform/Allocators/WeightedSIMappingFinder.h"
#include "enfield/Transform/CircuitGraphBuilderPass.h"
#include "enfield/Transform/PassCache.h"
#include "enfield/Support/BFSPathFinder.h"

#include <algorithm>

using namespace efd;

struct AllocProps {
    CircuitNode* cnode;
    bool mvTgtSrc;
    uint32_t cost;
    std::vector<uint32_t> path;
};

GreedyCktQAllocator::GreedyCktQAllocator(ArchGraph::sRef ag) : QbitAllocator(ag) {}

Solution GreedyCktQAllocator::executeAllocation(QModule::Ref qmod) {
    auto depPass = PassCache::Get<DependencyBuilderWrapperPass>(mMod);
    auto depBuilder = depPass->getData();
    auto& depsSet = depBuilder.getDependencies();
    auto nofDepsSet = depsSet.size();

    auto cgbpass = PassCache::Get<CircuitGraphBuilderPass>(qmod);
    auto cgraph = cgbpass->getData();
    auto xbits = cgraph.size();

    BFSPathFinder bfs;

    auto mapfinder = WeightedSIMappingFinder::Create();
    auto mapping = mapfinder->find(mArchGraph.get(), depsSet);
    auto assign = GenAssignment(mArchGraph->size(), mapping);

    Solution sol { mapping, Solution::OpSequences(depsSet.size()), 0 };

    std::vector<Node::uRef> newNodes;
    std::map<CircuitNode*, uint32_t> reached;
    std::vector<bool> marked(xbits, false);
    std::vector<bool> frozen(mapping.size(), false);

    for (uint32_t t = 0; t < nofDepsSet; ++t) {

        bool changed;
        do {
            changed = false;

            for (uint32_t i = 0; i < xbits; ++i) {
                auto cnode = cgraph[i];

                if (cnode && cnode->qargsid.size() + cnode->cargsid.size() <= 1) {
                    newNodes.push_back(cnode->node->clone());
                    cgraph[i] = cnode->child[i];
                    changed = true;
                }
            }
        } while (changed);

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
        AllocProps bestProps { nullptr, false, _undef, std::vector<uint32_t>() };

        for (auto cnode : allocatable) {
            // Calculate cost for allocating cnode->node;

            auto node = cnode->node;
            auto dep = depBuilder.getDeps(node);

            assert(dep.getSize() <= 1 && "Can only allocate gates with at most one depenency.");
            cnode->node->print(std::cout, true);

            uint32_t a = dep[0].mFrom, b = dep[0].mTo;
            uint32_t u = mapping[a], v = mapping[b];

            auto bfspath = bfs.find(mArchGraph.get(), u, v);
            uint32_t pathsize = bfspath.size();
            AllocProps props { cnode, true, (pathsize - 2) * SwapCost.getVal(), bfspath };

            bool hasFrozenVertex = false;
            for (uint32_t u : bfspath) {
                if (frozen[u]) { hasFrozenVertex = true; break; }
            }

            if (hasFrozenVertex) {
                bool hasEdgeFromU = mArchGraph->hasEdge(bfspath[0], bfspath[1]);
                bool hasEdgeToV = mArchGraph->hasEdge(bfspath[pathsize - 2], bfspath[pathsize - 1]);

                if (!hasEdgeFromU && hasEdgeToV)
                    props.mvTgtSrc = false;
                else if (!hasEdgeFromU && !hasEdgeToV)
                    props.cost += RevCost.getVal();

                if (bestProps.cost > props.cost)
                    bestProps = props;
            }
        }

        assert(bestProps.cnode != nullptr && "There must be a 'best' node.");

        // Allocate best node;
        // Setting the 'stop' flag;
        auto& ops = sol.mOpSeqs[t];
        auto node = bestProps.cnode->node;
        auto newNode = node->clone();

        ops.first = newNode.get();
        newNodes.push_back(std::move(newNode));

        if (bestProps.mvTgtSrc)
            std::reverse(bestProps.path.begin(), bestProps.path.end());

        for (auto it = bestProps.path.begin() + 1, end = bestProps.path.end(); it != end; ++it) {
            uint32_t u = *(it - 1), v = *it;
            uint32_t a = assign[u], b = assign[v];

            ops.second.push_back({ Operation::K_OP_SWAP, a, b });

            std::swap(mapping[a], mapping[b]);
            std::swap(assign[u], assign[v]);
        }

        auto dep = depBuilder.getDeps(bestProps.cnode->node)[0];
        uint32_t a = dep.mFrom, b = dep.mTo;

        if (mArchGraph->hasEdge(mapping[a], mapping[b]))
            ops.second.push_back({ Operation::K_OP_CNOT, a, b });
        else
            ops.second.push_back({ Operation::K_OP_REV, a, b });

        for (uint32_t q : bestProps.cnode->qargsid)
            cgraph[q] = cgraph[q]->child[q];
        for (uint32_t c : bestProps.cnode->cargsid)
            cgraph[c] = cgraph[c]->child[c];

        sol.mCost += bestProps.cost;
    }

    qmod->clearStatements();
    for (auto& node : newNodes) {
        qmod->insertStatementLast(std::move(node));
    }

    return sol;
}

GreedyCktQAllocator::uRef GreedyCktQAllocator::Create(ArchGraph::sRef ag) {
    return uRef(new GreedyCktQAllocator(ag));
}
