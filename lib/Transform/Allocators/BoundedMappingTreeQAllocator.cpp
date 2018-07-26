#include "enfield/Transform/Allocators/BoundedMappingTreeQAllocator.h"
#include "enfield/Support/ApproxTSFinder.h"
#include "enfield/Support/CommandLine.h"
#include "enfield/Support/Defs.h"
#include "enfield/Support/Timer.h"

#include <algorithm>

using namespace efd;

static Opt<uint32_t> MaxChildren
("-bsi-max-children", "Limits the max number of children per partial solution.",
 std::numeric_limits<uint32_t>::max(), false);

static Opt<uint32_t> MaxPartialSolutions
("-bsi-max-partial", "Limits the max number of partial solutions per step.",
 std::numeric_limits<uint32_t>::max(), false);

namespace efd {
    struct Candidate;
}

BoundedMappingTreeQAllocator::BoundedMappingTreeQAllocator(ArchGraph::sRef ag)
    : QbitAllocator(ag),
      mDistance(mPQubits, Vector(mPQubits, _undef)) {
    preCalculateDistance();
}

Vector BoundedMappingTreeQAllocator::distanceFrom(uint32_t u) {
    uint32_t size = mArch->size();

    Vector distance(size, _undef);
    std::queue<uint32_t> q;
    std::vector<bool> visited(size, false);

    q.push(u);
    visited[u] = true;
    distance[u] = 0;

    while (!q.empty()) {
        uint32_t u = q.front();
        q.pop();

        for (uint32_t v : graph->adj(u)) {
            if (!visited[v]) {
                visited[v] = true;
                distance[v] = distance[u] + 1;
                q.push(v);
            }
        }
    }

    return distance;
}

void BoundedMappingTreeQAllocator::preCalculateDistance() {
    for (uint32_t i = 0; i < mPQubits; ++i) {
        mDistance[i] = distanceFrom(i, mArchGraph.get());
    }
}

void BoundedMappingTreeQAllocator::phase1() {
    CandidateVector candidates { { Mapping(mVQubits, _undef), 0 } };
    std::vector<CandidateVector> candidates;
    std::vector<bool> mapped(mVQubits, false);

    bool isFirst = true;

    // First Phase:
    //     in this phase, we divide the program in layers, such that each layer is satisfied
    //     by any of the mappings inside 'candidates'.
    for (auto& iDependencies : deps) {
        auto nofIDeps = iDependencies.getSize();
        if (nofIDeps > 1) {
            ERR << "Instructions with more than one dependency not supported "
                << "(" << iDependencies.mCallPoint->toString(false) << ")" << std::endl;
            std::exit(static_cast<uint32_t>(ExitCode::EXIT_multi_deps));
        } else if (nofIDeps < 1) {
            continue;
        }

        auto dep = iDependencies[0];
        auto newCandidates = extendCandidates(dep, mapped, candidates, isFirst);
        INF << "(first) Candidate number: " << newCandidates.size() << std::endl;

        if (newCandidates.empty()) {
            INF << "Reset!" << std::endl;
            INF << MappingToString(candidates[0].m) << std::endl;
            // Save candidates and reset!
            candidatesCollection.push_back(candidates);
            // Process this dependency again.
            candidates = { { Mapping(mVQubits, _undef), 0 } };
            mapped.assign(mVQubits, false);
            newCandidates = extendCandidates(dep, mapped, candidates, true);
        }

        isFirst = false;
        candidates = newCandidates;

        INF << "Dep (" << dep.mFrom << ", " << dep.mTo << ")" << std::endl;
        INF << "Candidate number: " << candidates.size() << std::endl;
    }
    candidatesCollection.push_back(candidates);
}

Solution BoundedMappingTreeQAllocator::executeAllocation(QModule::Ref qmod) {
    Solution sol;

    sol.mCost = 0;
    sol.mInitial.assign(mVQubits, 0);
    for (uint32_t i = 0; i < mVQubits; ++i) {
        sol.mInitial[i] = i;
    }


    // Second Phase:
    //     here, the idea is to use, perhaps, dynamic programming to test all possibilities
    //     for 'glueing' the sequence of candidatesCollection together.
    uint32_t nofLayers = candidatesCollection.size();
    uint32_t layerMaxSize = 0;

    for (uint32_t i = 0; i < nofLayers; ++i)
        layerMaxSize = std::max(layerMaxSize, (uint32_t) candidatesCollection[i].size());
    
    INF << "Dynamic Programming PHASE" << std::endl;
    INF << "Layers: " << nofLayers << std::endl;
    INF << "MaxSize: " << layerMaxSize << std::endl;

    if (nofLayers > 0) {
        std::vector<std::vector<bsi::TracebackInfo>> mem
            (nofLayers, std::vector<bsi::TracebackInfo>(layerMaxSize, { {}, _undef, _undef }));

        for (uint32_t i = 0, e = candidatesCollection[0].size(); i < e; ++i)
            mem[0][i] = { candidatesCollection[0][i].m, _undef, candidatesCollection[0][i].cost };

        for (uint32_t i = 1; i < nofLayers; ++i) {
            INF << "Beginning: " << i << " of " << nofLayers << " layers." << std::endl;
            uint32_t jLayerSize = candidatesCollection[i].size();
            for (uint32_t j = 0; j < jLayerSize; ++j) {
                Timer jt;
                jt.start();

                bsi::TracebackInfo best = { {}, _undef, _undef };
                uint32_t kLayerSize = candidatesCollection[i - 1].size();

                for (uint32_t k = 0; k < kLayerSize; ++k) {
                    auto candidate = candidatesCollection[i][j].m;
                    auto last = mem[i - 1][k].m;

                    uint32_t cost = estimateCost(last, candidate, distance);
                    cost += mem[i - 1][k].cost;

                    if (cost < best.cost) {
                        best = { candidate, k, cost };
                    }
                }

                mem[i][j] = best;

                jt.stop();
                INF << "(i:" << i << ", j:" << j << "): "
                    << ((double) jt.getMilliseconds()) / 1000.0 << std::endl;
            }

            INF << "End: " << i << " of " << nofLayers << " layers." << std::endl;
        }

        bsi::TracebackInfo best = { {}, _undef, _undef };
        uint32_t lastLayer = nofLayers - 1;

        for (uint32_t i = 0, e = candidatesCollection[lastLayer].size(); i < e; ++i) {
            if (mem[lastLayer][i].cost < best.cost)
                best = mem[lastLayer][i];
        }

        std::vector<bsi::TracebackInfo> infoVector;

        {
            // Here, we populate the stack of TracebackInfo.
            // Doing so, the one that is in the top is the one that should be used
            // sooner.
            bsi::TracebackInfo info = best;

            for (int32_t i = nofLayers - 1; i >= 0; --i) {
                infoVector.push_back(info);

                if (info.parent != _undef)
                    info = mem[i - 1][info.parent];
            }

            std::reverse(infoVector.begin(), infoVector.end());
        }

        // Third Phase:
        //     build the operations vector by tracebacking from the solution we have
        //     found. For this, we have to go through every dependency again.
        uint32_t idx = 0;
        auto info = infoVector[0];

        Mapping realToDummy = infoVector[0].m;
        Mapping dummyToPhys = IdentityMapping(mPQubits);
        for (auto& iDependencies : deps) {
            // We are sure that there are no instruction dependency that has more than
            // one dependency.
            if (iDependencies.getSize() < 1)
                continue;

            auto dep = iDependencies[0];

            uint32_t a = dep.mFrom, b = dep.mTo;
            uint32_t u = info.m[a], v = info.m[b];

            Solution::OpVector opVector;

            // If we can't satisfy (u, v) with the current mapping, it can only mean
            // that we must go to the next one.
            if ((u == _undef || v == _undef) ||
                (!mArchGraph->hasEdge(u, v) && !mArchGraph->hasEdge(v, u))) {

                if (++idx >= infoVector.size()) {
                    ERR << "Not enough mappings were generated, maybe!?" << std::endl;
                    ERR << "Mapping for '" << iDependencies.mCallPoint->toString(false)
                        << "'." << std::endl;
                    std::exit(static_cast<uint32_t>(ExitCode::EXIT_unreachable));
                }

                auto newInfo = infoVector[idx];

                // Transform all mappings into dummy mappings.
                auto prev = Mapping(mVQubits, _undef);
                auto curr = Mapping(mVQubits, _undef);

                for (uint32_t i = 0; i < mVQubits; ++i) {
                    if (info.m[i] != _undef && newInfo.m[i] == _undef) {
                        ERR << "Assumption that previous mappings have same mapped qubits "
                            << "than current mapping broken." << std::endl;
                        std::exit(static_cast<uint32_t>(ExitCode::EXIT_unreachable));
                    }

                    if (info.m[i] != _undef) {
                        prev[realToDummy[i]] = info.m[i];
                        curr[realToDummy[i]] = newInfo.m[i];
                    }
                }

                auto prevAssign = GenAssignment(mPQubits, prev, false);
                auto currAssign = GenAssignment(mPQubits, curr, false);

                ApproxTSFinder finder(mArchGraph);
                auto swaps = finder.find(prevAssign, currAssign);

                auto assign = GenAssignment(mPQubits, dummyToPhys);

                INF << "From " << MappingToString(info.m) << " to " << MappingToString(newInfo.m) << std::endl;

                for (auto swp : swaps) {
                    uint32_t a = assign[swp.u], b = assign[swp.v];

                    if (!mArchGraph->hasEdge(swp.u, swp.v)) {
                        std::swap(a, b);
                    }

                    std::swap(assign[swp.u], assign[swp.v]);
                    std::swap(dummyToPhys[a], dummyToPhys[b]);
                    opVector.push_back({ Operation::K_OP_SWAP, a, b });
                }

                sol.mCost += (SwapCost.getVal() * swaps.size());

                for (uint32_t i = 0; i < mVQubits; ++i) {
                    if (realToDummy[i] == _undef && newInfo.m[i] != _undef) {
                        realToDummy[i] = assign[newInfo.m[i]];
                    }
                }

                info = newInfo;
            }

            u = info.m[a];
            v = info.m[b];

            Operation op;
            op.mU = a;
            op.mV = b;

            if (mArchGraph->hasEdge(u, v)) {
                op.mK = Operation::K_OP_CNOT;
            } else if (mArchGraph->hasEdge(v, u)) {
                op.mK = Operation::K_OP_REV;
                sol.mCost += RevCost.getVal();
            } else {
                ERR << "Mapping " << MappingToString(info.m) << " not able to satisfy dependency "
                    << "(" << a << "{" << u << "}, " << b << "{" << v << "})" << std::endl;
                std::exit(static_cast<uint32_t>(ExitCode::EXIT_unreachable));
            }

            opVector.push_back(op);
            sol.mOpSeqs.push_back(std::make_pair(iDependencies.mCallPoint, opVector));
        }

        Fill(mVQubits, realToDummy);
        sol.mInitial = realToDummy;

        auto dummyToReal = GenAssignment(mVQubits, realToDummy, false);

        // Transforming swaps from dummy to real.
        for (auto& pair : sol.mOpSeqs) {
            for (auto& op : pair.second) {
                if (op.mK == Operation::K_OP_SWAP) {
                    op.mU = dummyToReal[op.mU];
                    op.mV = dummyToReal[op.mV];
                }
            }
        }
    } else {
        Fill(mVQubits, sol.mInitial);
    }

    return sol;
}

BoundedMappingTreeQAllocator::uRef
BoundedMappingTreeQAllocator::Create(ArchGraph::sRef ag) {
    return uRef(new BoundedMappingTreeQAllocator(ag));
}
