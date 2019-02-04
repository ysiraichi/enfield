#include "enfield/Transform/Allocators/ChallengeWinnerQAllocator.h"
#include "enfield/Transform/CircuitGraphBuilderPass.h"
#include "enfield/Transform/QubitRemapPass.h"
#include "enfield/Transform/Utils.h"
#include "enfield/Transform/PassCache.h"
#include "enfield/Support/Defs.h"

#include <algorithm>
#include <numeric>
#include <queue>

namespace efd {
namespace chw {

    struct AStarNode {
        Mapping mapping;
        InverseMap inverse;
        SwapSeq swaps;
        std::vector<bool> qUsed;
        uint32_t cost;
        bool finished;
    };

    struct AStarNodeCompare {
    	bool operator()(const AStarNode& lhs, const AStarNode& rhs) const {
            return lhs.cost > rhs.cost;
    	}
    };

}
}

using namespace efd;
using namespace chw;

ChallengeWinnerQAllocator::ChallengeWinnerQAllocator(ArchGraph::sRef archGraph)
    : QbitAllocator(archGraph) {}

uint32_t ChallengeWinnerQAllocator::getOrAssignPQubitFor(uint32_t a,
                                                         const Mapping& mapping,
                                                         const InverseMap& inverse) {
    if (mapping[a] == _undef) {
        for (uint32_t u = 0; u < mPQubits; ++u) {
            if (inverse[u] == _undef) return u;
        }

        EfdAbortIf(true, "No physical qubit available.");
    } else {
        return mapping[a];
    }
}

SwapSeq ChallengeWinnerQAllocator::astar(const std::vector<Dep>& dependencies,
                                         const Mapping& mapping,
                                         const InverseMap& inverse,
                                         const std::set<UIntPair>& freeSwaps) {

    using AStarPQueue = std::priority_queue<AStarNode,
                                            std::vector<AStarNode>,
                                            AStarNodeCompare>;

    std::set<uint32_t> usedQubits;

    for (auto& dep : dependencies) {
        usedQubits.insert(dep.mFrom);
        usedQubits.insert(dep.mTo);
    }

    AStarPQueue queue;
    queue.push(
            AStarNode {
                mapping,
                inverse,
                SwapSeq(),
                std::vector<bool>(mPQubits, false),
                0,
                false
            });

    while (!queue.top().finished) {
        auto top = queue.top();
        queue.pop();

        for (uint32_t a : usedQubits) {
            uint32_t u = top.mapping[a];

            for (uint32_t v : mArchGraph->adj(u)) {
                if (!top.swaps.empty() && top.swaps.back() == Swap { u, v }) continue;

                AStarNode child = top;
                child.swaps.push_back(Swap { u, v });

                std::swap(child.inverse[u], child.inverse[v]);
                if (child.inverse[u] != _undef) child.mapping[child.inverse[u]] = u;
                if (child.inverse[v] != _undef) child.mapping[child.inverse[v]] = v;

                if (child.qUsed[u] ||
                    child.qUsed[v] ||
                    freeSwaps.find(UIntPair(u, v)) == freeSwaps.end()) ++child.cost;

                child.qUsed[u] = true;
                child.qUsed[v] = true;

                for (auto& dep : dependencies) {
                    uint32_t dist = mDistance.get(child.mapping[dep.mFrom],
                                                  child.mapping[dep.mTo]);

                    if (dist == 1) child.finished = true;
                    child.cost += dist;
                }

                queue.push(child);
            }
        }
    }

    return queue.top().swaps;
}

Mapping ChallengeWinnerQAllocator::allocate(QModule::Ref qmod) {
    auto depPass = PassCache::Get<DependencyBuilderWrapperPass>(qmod);
    auto& depBuilder = depPass->getData();
    auto& xbitToN = depBuilder.getXbitToNumber();

    auto cgbpass = PassCache::Get<CircuitGraphBuilderPass>(qmod);
    auto cgraph = cgbpass->getData();
    auto it = cgraph.build_iterator();

    auto xbitNumber = cgraph.size();

    Mapping mapping(mVQubits, _undef);
    InverseMap inverse(mPQubits, _undef);

    std::vector<Node::uRef> allocatedStatements;
    std::map<Node::Ref, uint32_t> reached;

    QubitRemapVisitor visitor(mapping, xbitToN);

    mDistance.init(mArchGraph.get());

    for (uint32_t i = 0; i < xbitNumber; ++i) {
        it.next(i);
        ++reached[it.get(i)];
    }

    std::vector<UIntPair> appliedGates;
    std::set<UIntPair> freeSwaps;
    SwapSeq swapSequence;

    while (true) {
        std::set<CircuitGraph::CircuitNode::sRef> allocatable;
        std::vector<Dep> dependencies;
        bool changed, redo = false;

        do {
            changed = false;

            for (uint32_t i = 0; i < xbitNumber; ++i) {
                auto cnode = it[i];
                auto node = it.get(i);

                if (cnode->isGateNode()) {
                    auto dep = depBuilder.getDeps(node);

                    if (dep.size() == 0 && cnode->numberOfXbits() == reached[it.get(i)]) {
                        for (uint32_t i : cnode->getXbitsId()) {
                            if (i < mVQubits) {
                                mapping[i] = getOrAssignPQubitFor(i, mapping, inverse);
                                inverse[mapping[i]] = i;
                            }

                            it.next(i);
                            ++reached[it.get(i)];
                        }

                        auto clone = node->clone();
                        clone->apply(&visitor);
                        allocatedStatements.push_back(std::move(clone));

                        changed = true;
                    }
                }
            }
        } while (changed);

        // Advance the xbitNumber' cgraph and unmark them.
        for (uint32_t i = 0; i < xbitNumber; ++i) {
            auto cnode = it[i];
            auto node = cnode->node();

            if (cnode->isGateNode() && reached[node] == cnode->numberOfXbits()) {
                allocatable.insert(cnode);
            }
        }

        if (allocatable.empty()) break;

        for (auto cnode : allocatable) {
            auto node = cnode->node();
            auto deps = depBuilder.getDeps(node);
            EfdAbortIf(deps.size() != 1,
                       "Can only handle gates with one or less dependencies.");
            auto dep = deps[0];
            uint32_t a = dep.mFrom, b = dep.mTo;

            std::vector<uint32_t> aCandidates { mapping[a] };
            std::vector<uint32_t> bCandidates { mapping[b] };

            if (mapping[a] == _undef) {
                aCandidates.clear();
                for (uint32_t u = 0; u < mPQubits; ++u) {
                    if (inverse[u] == _undef) aCandidates.push_back(u);
                }
            }

            if (mapping[b] == _undef) {
                bCandidates.clear();
                for (uint32_t u = 0; u < mPQubits; ++u) {
                    if (inverse[u] == _undef) bCandidates.push_back(u);
                }
            }

            struct { uint32_t aQ, bQ, dist; } best;
            best.dist = _undef;
            best.aQ = _undef;
            best.bQ = _undef;

            for (auto aQ : aCandidates) {
                for (auto bQ : bCandidates) {
                    if (aQ == bQ) continue;

                    uint32_t dist = mDistance.get(aQ, bQ);

                    if (dist < best.dist) {
                        best.dist = dist;
                        best.aQ = aQ;
                        best.bQ = bQ;
                    }
                }
            }

            mapping[a] = best.aQ;
            mapping[b] = best.bQ;
            inverse[best.aQ] = a;
            inverse[best.bQ] = b;

            EfdAbortIf(best.dist == _undef,
                    "No available qubit found for '" << a << "'(" << mapping[a] <<
                    "), '" << b << "'(" << mapping[b] << ")");

            if (best.dist == 1) {
                if (!mArchGraph->hasEdge(best.aQ, best.bQ)) std::swap(best.aQ, best.bQ);
                appliedGates.push_back(UIntPair(best.aQ, best.bQ));

                for (uint32_t i : cnode->getXbitsId()) {
                    it.next(i);
                    ++reached[it.get(i)];
                }

                auto clone = node->clone();
                clone->apply(&visitor);
                allocatedStatements.push_back(std::move(clone));
                redo = true;
            } else {
                dependencies.push_back(dep);
            }
        }

        if (redo) continue;

        std::vector<bool> usedQubits(mPQubits, false);
        for (auto& gate : appliedGates) {
            if (!usedQubits[gate.first] && !usedQubits[gate.second]) {
                freeSwaps.insert(gate);
            }

            usedQubits[gate.first] = true;
            usedQubits[gate.second] = true;
        }

        auto swaps = astar(dependencies, mapping, inverse, freeSwaps);
        swapSequence.insert(swapSequence.end(), swaps.begin(), swaps.end());

        for (auto swap : swaps) {
            allocatedStatements.push_back(
                    CreateISwap(mArchGraph->getNode(swap.u)->clone(),
                                mArchGraph->getNode(swap.v)->clone()));

            uint32_t a = inverse[swap.u];
            uint32_t b = inverse[swap.v];

            if (a != _undef) mapping[a] = swap.v;
            if (b != _undef) mapping[b] = swap.u;
            std::swap(inverse[swap.u], inverse[swap.v]);
        }

        freeSwaps.clear();
    }

    qmod->clearStatements();
    qmod->insertStatementLast(std::move(allocatedStatements));

    Fill(mVQubits, mapping);
    Fill(mPQubits, inverse);
    std::reverse(swapSequence.begin(), swapSequence.end());
    for (auto& swap : swapSequence) {
        uint32_t a = inverse[swap.u], b = inverse[swap.v];
        std::swap(inverse[swap.u], inverse[swap.v]);
        std::swap(mapping[a], mapping[b]);
    }

    return mapping;
}

ChallengeWinnerQAllocator::uRef ChallengeWinnerQAllocator::Create(ArchGraph::sRef ag) {
    return uRef(new ChallengeWinnerQAllocator(ag));
}
