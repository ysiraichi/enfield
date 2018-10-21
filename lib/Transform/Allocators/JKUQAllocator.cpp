#include "enfield/Transform/Allocators/JKUQAllocator.h"
#include "enfield/Transform/LayersBuilderPass.h"
#include "enfield/Transform/QubitRemapPass.h"
#include "enfield/Transform/PassCache.h"
#include "enfield/Support/Defs.h"
#include "enfield/Support/RTTI.h"
#include "enfield/Support/uRefCast.h"

using namespace efd;
using namespace jku;

namespace efd {
namespace jku {

    struct AStarNode {
        uint32_t costFixed = 0;
        uint32_t costTableHeur = 0;
        uint32_t costNextHeur = 0;
        uint32_t depth = 0;
        Mapping m;
        InverseMap inv;
        std::vector<Swap> swaps;
        bool finished = true;
    };

    struct AStarNodeCompare {
    	bool operator()(const AStarNode& lhs, const AStarNode& rhs) const {
            uint32_t lhsTotal = lhs.costFixed + lhs.costTableHeur + lhs.costNextHeur;
            uint32_t rhsTotal = rhs.costFixed + rhs.costTableHeur + rhs.costNextHeur;
            if (lhsTotal != rhsTotal) return lhsTotal > rhsTotal;
            if (lhs.finished) return false;
            if (rhs.finished) return true;
            return lhsTotal - lhs.costFixed > rhsTotal - rhs.costFixed;
    	}
    };

    using AStarPQueue = std::priority_queue<AStarNode, std::vector<AStarNode>, AStarNodeCompare>;

    struct ExpandNodeState {
        AStarPQueue& queue;
        const std::vector<uint32_t>& qubitsInLayer;
        std::vector<bool>& processed;
        std::vector<Swap> swaps;
        const Layers& layers;
        const uint32_t currentLayer;
        const uint32_t nextLayer;
    };
}
}

JKUQAllocator::JKUQAllocator(ArchGraph::sRef archGraph)
    : QbitAllocator(archGraph) {}

void JKUQAllocator::buildCostTable() {
    mTable.assign(mPQubits, std::vector<uint32_t>(mPQubits, 0));

    for (uint32_t u = 0; u < mPQubits; ++u) {
        for (uint32_t v = 0; v < mPQubits; ++v) {
            if (u == v) mTable[u][v] = 0;
            else {
                auto path = mBFS.find(mArchGraph.get(), u, v);
                mTable[u][v] = (path.size() - 2) * 7;

                bool onlyRevEdge = true;
                for (uint32_t i = 0, e = path.size(); i < e - 1; ++i) {
                    if (mArchGraph->hasEdge(path[i], path[i + 1])) {
                        onlyRevEdge = false;
                        break;
                    }
                }

                if (onlyRevEdge) {
                    mTable[u][v] += 4;
                }
            }
        }
    }
}

void JKUQAllocator::expandNodeRecursively(const AStarNode& aNode,
                                          uint32_t i,
                                          ExpandNodeState& state) {
    if (i == state.qubitsInLayer.size()) {
        if (state.swaps.empty()) return;

        AStarNode newNode;
        newNode.m = aNode.m;
        newNode.inv = aNode.inv;
        newNode.swaps = aNode.swaps;
        newNode.depth = aNode.depth + 5;
        newNode.costFixed = aNode.costFixed + 7 * state.swaps.size();
        newNode.finished = true;

        for (auto& s : state.swaps) {
            uint32_t a = newNode.inv[s.u], b = newNode.inv[s.v];
            if (a != _undef) newNode.m[a] = s.v;
            if (b != _undef) newNode.m[b] = s.u;
            std::swap(newNode.inv[s.u], newNode.inv[s.v]);
        }

        newNode.swaps.insert(newNode.swaps.end(), state.swaps.begin(), state.swaps.end());

        for (auto node : state.layers[state.currentLayer]) {
            auto deps = mDBuilder.getDeps(node);
            if (!deps.empty()) {
                auto dep = deps[0];
                auto cost = mTable[newNode.m[dep.mFrom]][newNode.m[dep.mTo]];
                newNode.costTableHeur += cost;
                newNode.finished = newNode.finished && cost <= 4;
            }
        }

        if (state.nextLayer != _undef) {
            for (auto node : state.layers[state.nextLayer]) {
                auto deps = mDBuilder.getDeps(node);

                if (deps.empty()) continue;

                uint32_t a = deps[0].mFrom, b = deps[0].mTo;
                uint32_t u = newNode.m[a], v = newNode.m[b];
                uint32_t costHeur = 0;

                if (u != v && (u == _undef || v == _undef)) {
                    uint32_t mapped;

                    if (u == _undef) {
                        mapped = b;
                    } else {
                        mapped = a;
                    }

                    uint32_t u = newNode.m[mapped];
                    uint32_t minCost = _undef;
                    for (uint32_t v = 0; v < mPQubits; ++v) {
                        auto cost = mTable[u][v];
                        if (newNode.inv[v] == _undef && cost < minCost) {
                            minCost = cost;
                        }
                    }

                    costHeur = minCost; 
                } else if (u != v) {
                    costHeur =  mTable[u][v];
                }

                newNode.costNextHeur += costHeur;
            }
        }

        state.queue.push(newNode);
    } else {
        expandNodeRecursively(aNode, i + 1, state);

        uint32_t _u = aNode.m[state.qubitsInLayer[i]];
        if (!state.processed[_u]) {
            for (auto _v : mArchGraph->adj(_u)) {
                if (!state.processed[_v]) {
                    uint32_t u = _u, v = _v;
                    if (!mArchGraph->hasEdge(u, v)) std::swap(u, v);
                    state.processed[u] = true;
                    state.processed[v] = true;
                    state.swaps.push_back(Swap { u, v });

                    expandNodeRecursively(aNode, i + 1, state);

                    state.swaps.pop_back();
                    state.processed[u] = false;
                    state.processed[v] = false;
                }
            }
        }
    }
}

AStarNode JKUQAllocator::astar(std::queue<uint32_t>& cnotLayersIdQ,
                               const Layers& layers,
                               uint32_t i,
                               Mapping& mapping,
                               InverseMap& inverse) {
    uint32_t nextLayer;

    if (!cnotLayersIdQ.empty() && cnotLayersIdQ.front() == i) {
        cnotLayersIdQ.pop();
    }

    if (!cnotLayersIdQ.empty()) nextLayer = cnotLayersIdQ.front();
    else nextLayer = _undef;

    std::vector<uint32_t> qubitsInLayer;
    uint32_t maxCost = 0;

    for (auto& node : layers[i]) {
        auto deps = mDBuilder.getDeps(node);
        if (deps.empty()) continue;

        uint32_t a = deps[0].mFrom, b = deps[0].mTo;
        qubitsInLayer.push_back(a);
        qubitsInLayer.push_back(b);

        if (mapping[a] == _undef && mapping[b] == _undef) {
            std::set<std::pair<uint32_t, uint32_t>> possibleEdges;

            for (uint32_t u = 0; u < mPQubits; ++u) {
                if (inverse[u] != _undef) continue;

                for (uint32_t v : mArchGraph->succ(u)) {
                    if (inverse[v] != _undef) continue;
                    possibleEdges.insert(std::make_pair(u, v));
                }
            }

            if (!possibleEdges.empty()) {
                auto& pair = *possibleEdges.begin();
                mapping[a] = pair.first;
                mapping[b] = pair.second;
                inverse[pair.first] = a;
                inverse[pair.second] = b;
            } else {
                EfdAbortIf(true, "No available edges!");
            }
        } else if (mapping[a] == _undef || mapping[b] == _undef) {
            uint32_t unmapped;
            uint32_t mapped;

            if (mapping[a] == _undef) {
                unmapped = a;
                mapped = b;
            } else {
                unmapped = b;
                mapped = a;
            }

            uint32_t minCost = _undef;
            uint32_t minVertex = _undef;

            uint32_t u = mapping[mapped];
            for (uint32_t v = 0; v < mPQubits; ++v) {
                uint32_t cost;
                if (u == mapping[a]) cost = mTable[u][v];
                else cost = mTable[v][u];

                if (inverse[v] == _undef && cost < minCost) {
                    minCost = cost;
                    minVertex = v;
                }
            }

            mapping[unmapped] = minVertex;
            inverse[minVertex] = unmapped;

        }
        // The cost distance heuristic is the maximum distance from
        // the vertices that should be together.
        maxCost = std::max(maxCost, mTable[mapping[a]][mapping[b]]);
    }

    AStarNode aNode;
    aNode.m = mapping;
    aNode.inv = inverse;
    aNode.finished = (maxCost <= 4);
    aNode.costTableHeur = maxCost;

    AStarPQueue astarQ;
    astarQ.push(aNode);

    while (!astarQ.top().finished) {
        auto aNode = astarQ.top();
        astarQ.pop();

        std::vector<bool> processed(mVQubits, false);

        ExpandNodeState state {
            astarQ,
            qubitsInLayer,
            processed,
            std::vector<Swap>(),
            layers,
            i,
            nextLayer
        };

        expandNodeRecursively(aNode, 0, state);
    }

    return astarQ.top();
}

Mapping JKUQAllocator::allocate(QModule::Ref qmod) {
    buildCostTable();

    auto layers = PassCache::Get<LayersBuilderPass>(qmod)->getData();
    mDBuilder = PassCache::Get<DependencyBuilderWrapperPass>(qmod)->getData();
    auto& xbitToN = mDBuilder.getXbitToNumber();

    Mapping mapping(mVQubits, _undef);
    InverseMap inverse(mPQubits, _undef);

    // Storing all layers index that have at least a CNOT gate.
    std::queue<uint32_t> cnotLayersIdQ;
    for (uint32_t i = 0, e = layers.size(); i < e; ++i) {
        const auto& layer = layers[i];
        for (const auto& node : layer) {
            if (!mDBuilder.getDeps(node).empty()) {
                cnotLayersIdQ.push(i);
                break;
            }
        }
    }

    std::vector<Node::Ref> unmappedSingeQubitGate;
    std::vector<Node::uRef> newStatements;

    QubitRemapVisitor visitor(mapping, xbitToN);

    for (uint32_t i = 0, e = layers.size(); i < e; ++i) {
        auto aNode = astar(cnotLayersIdQ, layers, i, mapping, inverse);

        mapping = aNode.m;
        inverse = aNode.inv;

        if (i != 0) {
            for (const auto& s : aNode.swaps) {
                newStatements.push_back(CreateISwap(mArchGraph->getNode(s.u)->clone(),
                                                    mArchGraph->getNode(s.v)->clone()));
            }
        }

        for (auto node : layers[i]) {
            auto deps = mDBuilder.getDeps(node);
            auto clone = node->clone();

            if (deps.empty()) {
                clone->apply(&visitor);

                if (!visitor.wasReplaced()) {
                    unmappedSingeQubitGate.push_back(clone.get());
                }
            } else {
                auto dep = deps[0];
                uint32_t u = mapping[dep.mFrom], v = mapping[dep.mTo];

                if (mArchGraph->hasEdge(u, v)) {
                    clone->apply(&visitor);
                } else {
                    auto sPair = GetStatementPair(clone.get());
                    auto qop = CreateIRevCX(mArchGraph->getNode(u)->clone(),
                                            mArchGraph->getNode(v)->clone());

                    if (sPair.first != nullptr) {
                        auto ifstmt = dynCast<NDIfStmt>(clone.get());
                        ifstmt->setQOp(std::move(qop));
                    } else {
                        clone = std::move(qop);
                    }
                }
            }

            newStatements.push_back(std::move(clone));
        }
    }

    auto singleQubitGateIt = unmappedSingeQubitGate.rbegin();
    auto singleQubitGateEnd = unmappedSingeQubitGate.rend();

    for (auto it = newStatements.rbegin(), end = newStatements.rend(); it != end; ++it) {
        auto node = it->get();
        auto sPair = GetStatementPair(node);
        
        if (IsIntrinsicGateCall(sPair.second) &&
            GetIntrinsicKind(sPair.second) == NDQOpGen::IntrinsicKind::K_INTRINSIC_SWAP) {
            auto qargs = sPair.second->getQArgs();
            uint32_t u = mArchGraph->getUId(qargs->getChild(0)->toString(false));
            uint32_t v = mArchGraph->getUId(qargs->getChild(1)->toString(false));

            uint32_t a = inverse[u], b = inverse[v];
            if (a != _undef) mapping[a] = v;
            if (b != _undef) mapping[b] = u;
            std::swap(inverse[u], inverse[v]);
        } else if (singleQubitGateIt != singleQubitGateEnd && node == *singleQubitGateIt) {
            ++singleQubitGateIt;
            auto qargs = sPair.second->getQArgs();

            for (const auto& q : *qargs) {
                uint32_t a = xbitToN.getQUId(q->toString(false));

                if (mapping[a] == _undef) {
                    for (uint32_t u = 0; u < mPQubits; ++u) {
                        if (inverse[u] == _undef) {
                            mapping[a] = u;
                            inverse[u] = a;
                            break;
                        }
                    }
                }
            }

            node->apply(&visitor);
        }
    }

    qmod->clearStatements();
    qmod->insertStatementLast(std::move(newStatements));

    Fill(mPQubits, mapping);
    return mapping;
}

JKUQAllocator::uRef JKUQAllocator::Create(ArchGraph::sRef archGraph) {
    return uRef(new JKUQAllocator(archGraph));
}
