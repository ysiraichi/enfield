#include "enfield/Transform/Allocators/IBMQAllocator.h"
#include "enfield/Transform/PassCache.h"
#include "enfield/Support/BFSPathFinder.h"

#include <random>

using namespace efd;

extern Opt<uint32_t> Seed;
static Opt<uint32_t> Trials
("trials", "Number of times that IBMQAllocator should try.", 20, false);

IBMQAllocator::IBMQAllocator(ArchGraph::sRef archGraph) : QbitAllocator(archGraph) {}

IBMQAllocator::uRef IBMQAllocator::Create(ArchGraph::sRef archGraph) {
    return uRef(new IBMQAllocator(archGraph));
}

IBMQAllocator::AllocationResult IBMQAllocator::tryAllocateLayer
(Layer& layer, Mapping current, std::set<uint32_t> qubitsSet, DependencyBuilder& depData) {
    AllocationResult result { current, true, {}, false };
    Assign assign = GenAssignment(mPQubits, current);

    std::default_random_engine generator(Seed.getVal());
    std::normal_distribution<double> distribution(0.0, (double) (1 / (double) mPQubits));

    std::vector<Dep> deps;
    for (auto node : layer) {
        auto _deps = depData.getDeps(node);

        if (_deps.getSize() > 1) {
            ERR << "Not suporting gates with more than 1 dependency ("
                << node->toString(false) << ")." << std::endl;
            ExitWith(ExitCode::EXIT_unreachable);
        } else if (_deps.getSize() == 1) {
            deps.push_back(_deps[0]);
        }
    }

    uint32_t dist = 0;
    for (auto dep : deps) {
        uint32_t u = current[dep.mFrom], v = current[dep.mTo];
        dist += mDist[u][v];
    }

    if (dist == deps.size()) {
        result.isTrivialLayer = deps.empty();
        result.success = true; 
        return result;
    }

    uint32_t bestD = _undef;
    Mapping bestMap;
    Solution::OpVector bestOpv;
    bool found = false;

    uint32_t trials = Trials.getVal();
    for (uint32_t i = 0; i < trials; ++i) {

        auto trialMap = current;
        auto trialAssign = assign;
        Solution::OpVector trialOpv;

        std::vector<std::vector<double>> rDist(mPQubits, std::vector<double>(mPQubits, _undef));
        for (uint32_t i = 0; i < mPQubits; ++i)
            for (uint32_t j = 0; j < mPQubits; ++j) {
                double scale = 1 + distribution(generator);
                rDist[i][j] = scale * mDist[i][j] * mDist[i][j];
                rDist[j][i] = rDist[i][j];
            }

        uint32_t d = 1;
        uint32_t maxD = (2 * mPQubits) + 1;
        while (d < maxD) {
            std::set<uint32_t> qubitSet = qubitsSet;

            auto optMap = trialMap;
            auto optAssign = trialAssign;
            Operation optOp;
            std::pair<uint32_t, uint32_t> optEdge;

            while (!qubitSet.empty()) {
                double minCost = 0;
                for (auto dep : deps) {
                    minCost += rDist[trialMap[dep.mFrom]][trialMap[dep.mTo]];
                }

                bool progress = false;
                for (uint32_t u = 0, endU = mArchGraph->size(); u < endU; ++u) {
                    for (uint32_t v : mArchGraph->adj(u)) {
                        bool hasU = qubitSet.find(u) != qubitSet.end();
                        bool hasV = qubitSet.find(v) != qubitSet.end();

                        if (hasU && hasV) {
                            // 1. Try a swap on (u, v)
                            auto _map = trialMap;
                            auto _assign = trialAssign;
                            std::swap(_map[_assign[u]], _map[_assign[v]]);
                            std::swap(_assign[u], _assign[v]);
                            // 2. Compute cost again
                            double _cost = 0;
                            for (auto dep : deps) {
                                _cost += rDist[_map[dep.mFrom]][_map[dep.mTo]];
                            }
                            // 3. Check if it is better than what we had.
                            //     3.1 If it is, update the minCost and other values.
                            if (minCost > _cost) {
                                progress = true;
                                minCost = _cost;
                                optMap = _map;
                                optAssign = _assign;
                                optEdge = std::make_pair(u, v);
                                optOp = { Operation::K_OP_SWAP, _assign[u], _assign[v] };
                            }
                        }
                    }
                }

                if (progress) {
                    qubitSet.erase(optEdge.first);
                    qubitSet.erase(optEdge.second);
                    trialMap = optMap;
                    trialAssign = optAssign;
                    trialOpv.push_back(optOp);
                } else {
                    break;
                }
            }

            uint32_t dist = 0;
            for (auto dep : deps) {
                uint32_t u = trialMap[dep.mFrom], v = trialMap[dep.mTo];
                dist += mDist[u][v];
            }

            if (dist == deps.size()) {
                break;
            }

            ++d;
        }

        uint32_t dist = 0;
        for (auto dep : deps) {
            uint32_t u = trialMap[dep.mFrom], v = trialMap[dep.mTo];
            dist += mDist[u][v];
        }

        if (dist == deps.size() && d < bestD) {
            found = true;
            bestMap = trialMap;
            bestOpv = trialOpv;
            bestD = d;
        }
    }

    if (found) {
        result.success = true;
        result.opv = bestOpv;
        result.map = bestMap;
    } else {
        result.success = false;
    }

    return result;
}

Solution IBMQAllocator::executeAllocation(QModule::Ref qmod) {
    std::srand(Seed.getVal());

    Solution sol;
    sol.mCost = 0;

    auto dbwPass = PassCache::Get<DependencyBuilderWrapperPass>(qmod);
    auto depData = dbwPass->getData();

    auto lbPass = PassCache::Get<LayersBuilderPass>(qmod);
    auto layers = lbPass->getData();

    BFSPathFinder bfs;

    mPQubits = mArchGraph->size();
    mLQubits = depData.mXbitToNumber.getQSize();
    mDist.assign(mPQubits, std::vector<uint32_t>(mPQubits, _undef));

    for (uint32_t i = 0; i < mPQubits; ++i) {
        for (uint32_t j = 0; j < mPQubits; ++j) {
            mDist[i][j] = bfs.find(mArchGraph.get(), i, j).size() - 1;
        }
    }

    Mapping current(mPQubits, 0);
    std::vector<bool> allocated(mPQubits, false);
    for (uint32_t i = 0, u = 0, endU = mArchGraph->size(); u < endU && i < mPQubits; ++u) {
        if (!allocated[u]) { current[i++] = u; allocated[u] = true; }

        for (uint32_t v : mArchGraph->succ(u)) {
            if (!allocated[v]) { current[i++] = v; allocated[v] = true; }
            if (i >= mPQubits) break;
        }
    }

    std::set<uint32_t> qubitsSet;
    for (uint32_t i = 0; i < mLQubits; ++i)
        qubitsSet.insert(i);

    std::vector<Node::uRef> newStatements;
    bool firstLayer = true;

    for (uint32_t i = 0, e = layers.size(); i < e; ++i) {
        auto& layer = layers[i];

        auto result = tryAllocateLayer(layer, current, qubitsSet, depData);

        if (result.success) {
            current = result.map;

            if (firstLayer) {
                sol.mInitial = current;
                result.opv.clear();
                firstLayer = false;
            }

            uint32_t opsSeqIdx = sol.mOpSeqs.size();

            for (auto it = layer.begin(), end = layer.end(); it != end; ++it) {
                auto node = *it;
                auto deps = depData.getDeps(node);

                auto clone = node->clone();

                if (deps.getSize() == 1) {
                    auto dep = deps[0];
                    uint32_t u = result.map[dep.mFrom], v = result.map[dep.mTo];

                    Operation op;

                    if (mArchGraph->hasEdge(u, v)) {
                        op = { Operation::K_OP_CNOT, dep.mFrom, dep.mTo };
                    } else if (mArchGraph->hasEdge(v, u)) {
                        op = { Operation::K_OP_REV, dep.mFrom, dep.mTo };
                        sol.mCost += RevCost.getVal();
                    } else {
                        ERR << "If we found one configuration, it should not reach this point."
                            << std::endl;
                        ExitWith(ExitCode::EXIT_unreachable);
                    }

                    sol.mOpSeqs.push_back(std::make_pair(clone.get(), Solution::OpVector{ op }));
                }

                newStatements.push_back(std::move(clone));
            }

            if (sol.mOpSeqs.size() != opsSeqIdx) {
                sol.mCost += (SwapCost.getVal() * result.opv.size());
                sol.mOpSeqs[opsSeqIdx].second.insert(sol.mOpSeqs[opsSeqIdx].second.begin(),
                                                     result.opv.begin(), result.opv.end());
            } else if (!result.opv.empty()) {
                ERR << "Swap operations but there were no dependencies to satisfy." << std::endl;
                ExitWith(ExitCode::EXIT_unreachable);
            }
        } else {
            INF << "Serializing this layer!" << std::endl;

            for (auto node : layer) {
                Layer sublayer { node };

                auto result = tryAllocateLayer(sublayer, current, qubitsSet, depData);

                if (!result.success) {
                    ERR << "Could not allocate sublayer " << node->toString(false)
                        << " on mapping: " << MappingToString(current) << "." << std::endl;
                    ExitWith(ExitCode::EXIT_unreachable);
                }

                current = result.map;

                if (firstLayer) {
                    sol.mInitial = current;
                    result.opv.clear();
                    firstLayer = false;
                }

                auto deps = depData.getDeps(node);
                auto clone = node->clone();

                Solution::OpVector opVector;

                if (!result.opv.empty()) {
                    opVector = result.opv;
                }

                if (deps.getSize() == 1) {
                    auto dep = deps[0];
                    uint32_t u = result.map[dep.mFrom], v = result.map[dep.mTo];

                    Operation op;

                    if (mArchGraph->hasEdge(u, v)) {
                        op = { Operation::K_OP_CNOT, dep.mFrom, dep.mTo };
                    } else if (mArchGraph->hasEdge(v, u)) {
                        op = { Operation::K_OP_REV, dep.mFrom, dep.mTo };
                        sol.mCost += RevCost.getVal();
                    } else {
                        ERR << "If we found one configuration, it should not reach this point."
                            << std::endl;
                        ExitWith(ExitCode::EXIT_unreachable);
                    }

                    opVector.push_back(op);
                }

                if (!opVector.empty()) {
                    sol.mOpSeqs.push_back(std::make_pair(clone.get(), opVector));
                }

                newStatements.push_back(std::move(clone));

                if (result.isTrivialLayer) continue;
            }
        }
    }

    if (firstLayer) {
        ERR << "No first layer found." << std::endl;
        ExitWith(ExitCode::EXIT_unreachable);
    }

    qmod->clearStatements();
    for (auto& node : newStatements) {
        qmod->insertStatementLast(std::move(node));
    }

    return sol;
}
