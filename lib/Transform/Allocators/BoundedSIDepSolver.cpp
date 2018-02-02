#include "enfield/Transform/Allocators/BoundedSIDepSolver.h"
#include "enfield/Support/ApproxTSFinder.h"
#include "enfield/Support/CommandLine.h"
#include "enfield/Support/Defs.h"
#include "enfield/Support/Timer.h"

#include <cstdlib>
#include <algorithm>
#include <queue>
#include <stack>

using namespace efd;

static Opt<uint32_t> MaxChildren
("-bsi-max-children", "Limits the max number of children per partial solution.",
 std::numeric_limits<uint32_t>::max(), false);

static Opt<uint32_t> MaxPartialSolutions
("-bsi-max-partial", "Limits the max number of partial solutions per step.",
 std::numeric_limits<uint32_t>::max(), false);

BoundedSIDepSolver::BoundedSIDepSolver(ArchGraph::sRef archGraph)
    : DepSolverQAllocator(archGraph) {}

BoundedSIDepSolver::uRef BoundedSIDepSolver::Create(ArchGraph::sRef archGraph) {
    return uRef(new BoundedSIDepSolver(archGraph));
}

struct TracebackInfo {
    Mapping m;
    uint32_t parent;
    uint32_t cost;
};

Solution BoundedSIDepSolver::solve(DepsSet& deps) {
    Solution sol;
    sol.mCost = 0;

    sol.mInitial.assign(mVQubits, 0);
    for (uint32_t i = 0; i < mVQubits; ++i)
        sol.mInitial[i] = i;

    CandidatesTy candidates { { Mapping(mVQubits, _undef), 0 } };
    std::vector<CandidatesTy> candidatesCollection;
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
        std::vector<std::vector<TracebackInfo>> mem
            (nofLayers, std::vector<TracebackInfo>(layerMaxSize, { {}, _undef, _undef }));

        for (uint32_t i = 0, e = candidatesCollection[0].size(); i < e; ++i)
            mem[0][i] = { candidatesCollection[0][i].m, _undef, candidatesCollection[0][i].cost };

        for (uint32_t i = 1; i < nofLayers; ++i) {
            INF << "Beginning: " << i << " of " << nofLayers << " layers." << std::endl;
            uint32_t jLayerSize = candidatesCollection[i].size();
            for (uint32_t j = 0; j < jLayerSize; ++j) {
                Timer jt;

                jt.start();
                TracebackInfo best = { {}, _undef, _undef };
                uint32_t kLayerSize = candidatesCollection[i - 1].size();

                for (uint32_t k = 0; k < kLayerSize; ++k) {
                    auto r = process(mem[i - 1][k].m, candidatesCollection[i][j].m);

                    uint32_t newCost = mem[i - 1][k].cost + candidatesCollection[i][j].cost
                        + (r.swaps.size() * SwapCost.getVal());

                    if (newCost < best.cost)
                        best = { r.newCurrent, k, newCost };
                }

                mem[i][j] = best;
                jt.stop();

                // INF << "(i:" << i << ", j:" << j << "): "
                //     << ((double) jt.getMilliseconds()) / 1000.0 << std::endl;
            }

            INF << "End: " << i << " of " << nofLayers << " layers." << std::endl;
        }

        TracebackInfo best = { {}, _undef, _undef };
        uint32_t lastLayer = nofLayers - 1;

        for (uint32_t i = 0, e = candidatesCollection[lastLayer].size(); i < e; ++i) {
            if (mem[lastLayer][i].cost < best.cost)
                best = mem[lastLayer][i];
        }

        sol.mCost = best.cost;
        std::stack<TracebackInfo> infoStack;

        {
            // Here, we populate the stack of TracebackInfo.
            // Doing so, the one that is in the top is the one that should be used
            // sooner.
            TracebackInfo info = best;

            for (int32_t i = nofLayers - 1; i >= 0; --i) {
                infoStack.push(info);

                if (info.parent != _undef)
                    info = mem[i - 1][info.parent];
            }
        }

        // Third Phase:
        //     build the operations vector by tracebacking from the solution we have
        //     found. For this, we have to go through every dependency again.
        auto info = infoStack.top();
        infoStack.pop();

        sol.mInitial = info.m;
        for (auto& iDependencies : deps) {
            // Here, we are sure that there are no instruction dependency that has more than
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

                if (infoStack.empty()) {
                    ERR << "Not enough mappings were generated, maybe!?" << std::endl;
                    ERR << "Mapping for '" << iDependencies.mCallPoint->toString(false)
                        << "'." << std::endl;
                    std::exit(static_cast<uint32_t>(ExitCode::EXIT_unreachable));
                }

                auto newInfo = infoStack.top();
                infoStack.pop();

                auto r = process(info.m, newInfo.m);

                auto last = r.newLast;
                auto assign = GenAssignment(mPQubits, r.newLast, false);

                for (auto swp : r.swaps) {
                    uint32_t a = assign[swp.u], b = assign[swp.v];
                    std::swap(last[a], last[b]);
                    std::swap(assign[swp.u], assign[swp.v]);
                    opVector.push_back({ Operation::K_OP_SWAP, a, b });
                }

                for (uint32_t i = 0; i < mVQubits; ++i)
                    if (sol.mInitial[i] == _undef && r.newLast[i] != _undef)
                        sol.mInitial[i] = r.newLast[i];

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
            } else {
                ERR << "Mapping " << MappingToString(info.m) << " not able to satisfy dependency "
                    << "(" << a << "{" << u << "}, " << b << "{" << v << "})" << std::endl;
                std::exit(static_cast<uint32_t>(ExitCode::EXIT_unreachable));
            }

            opVector.push_back(op);
            sol.mOpSeqs.push_back(std::make_pair(iDependencies.mCallPoint, opVector));
        }
    }

    auto initialAssign = GenAssignment(mPQubits, sol.mInitial);
    for (uint32_t i = 0; i < mVQubits; ++i)
        if (sol.mInitial[i] == _undef) {
            auto it = std::find(initialAssign.begin(), initialAssign.end(), i);
            sol.mInitial[i] = std::distance(initialAssign.begin(), it);
        }

    return sol;
}

uint32_t BoundedSIDepSolver::assignNonMappedVQubit(uint32_t u,
                                                   Assign& assign,
                                                   std::vector<bool>& notMapped) {
    uint32_t idx = 0;
    while (idx < mVQubits && !notMapped[idx]) ++idx;
    
    if (idx >= mVQubits) {
        ERR << "Not found virtual qubits for allocating." << std::endl;
        std::exit(static_cast<uint32_t>(ExitCode::EXIT_unreachable));
    }
    
    assign[u] = idx;
    notMapped[idx] = false;
    return idx;
}

BoundedSIDepSolver::TKSResult BoundedSIDepSolver::process(Mapping& last, Mapping& current) {
    Timer pt, ft;

    pt.start();
    TKSResult r { {}, current, last };
    auto assign = GenAssignment(mPQubits, r.newLast, false);

    // 1. We create a new 'last' mapping that will contain only the mappings relevant to
    // 'current'. This way we do not constrain the position of irrelevant qubits.
    for (uint32_t i = 0; i < mVQubits; ++i) {
        if (r.newCurrent[i] != _undef && r.newLast[i] == _undef) {
            r.newLast[i] = getNearest(r.newCurrent[i], assign);
            assign[r.newLast[i]] = i;
        } else if (r.newCurrent[i] == _undef) r.newLast[i] = _undef;
    }

    ApproxTSFinder finder(mArchGraph);
    auto lastAssign = GenAssignment(mPQubits, r.newLast, false);
    auto currentAssign = GenAssignment(mPQubits, r.newCurrent, false);

    ft.start();
    r.swaps = finder.find(lastAssign, currentAssign);
    ft.stop();

    // 2. Retrieval of the 'irrelevant' qubits, so that we maintain consistency.
    for (uint32_t i = 0; i < mVQubits; ++i) {
        if (last[i] != _undef) r.newLast[i] = last[i];
    }

    r.newCurrent = r.newLast;
    assign = GenAssignment(mPQubits, r.newCurrent, false);

    std::vector<bool> notMapped(mVQubits, false);

    for (uint32_t i = 0; i < mVQubits; ++i)
        if (r.newCurrent[i] == _undef)
            notMapped[i] = true;

    // 3. Application of the necessary swaps in order to reach 'current' with the irrelevant
    // qubits.
    for (auto swp : r.swaps) {
        std::array<std::pair<uint32_t, uint32_t>, 2> arr = {{
            { assign[swp.u], swp.u }, { assign[swp.v], swp.v }
        }};

        // If some of the vertices we are using for swaps are still not defined, we should
        // define those (up until now, we are doing it greedly)!
        for (auto& pair : arr) {
            if (pair.first != _undef) continue;
            pair.first = assignNonMappedVQubit(pair.second, assign, notMapped);
            r.newLast[pair.first] = pair.second;
            r.newCurrent[pair.first] = pair.second;
        }

        uint32_t a = arr[0].first, b = arr[1].first;
        uint32_t u = arr[0].second, v = arr[1].second;

        std::swap(r.newCurrent[a], r.newCurrent[b]);
        std::swap(assign[u], assign[v]);
    }

    pt.stop();
    // INF << "(" << ((double) ft.getMicroseconds()) / 1000000.0 << ", "
    //     << ((double) pt.getMicroseconds()) / 1000000.0 << ")" << std::endl;
    return r;
}

uint32_t BoundedSIDepSolver::getNearest(uint32_t u, Assign& assign) {
    std::vector<bool> visited(mPQubits, false);
    std::queue<uint32_t> q;
    q.push(u);
    visited[u] = true;

    while (!q.empty()) {
        uint32_t v = q.front();
        q.pop();

        if (assign[v] == _undef) return v;

        for (uint32_t w : mArchGraph->adj(v))
            if (!visited[w]) {
                visited[w] = true;
                q.push(w);
            }
    }

    // There is no way we can not find anyone!!
    ERR << "Can't find any vertice connected to v:" << u << "." << std::endl;
    std::exit(static_cast<uint32_t>(ExitCode::EXIT_unreachable));
}

BoundedSIDepSolver::CandidatesTy
BoundedSIDepSolver::extendCandidates(Dep& dep, std::vector<bool>& mapped,
                                     CandidatesTy& candidates, bool isFirst) {
    CandidatesTy newCandidates;
    uint32_t a = dep.mFrom, b = dep.mTo;
    uint32_t remainingSolutions = MaxPartialSolutions.getVal();

    bool bothUnmapped = !mapped[a] && !mapped[b];
    bool hasUnmapped = !mapped[a] || !mapped[b];

    // Of course, there is the possibility where both are mapped.
    // In this case, no worries since we do not use this variables.
    uint32_t unmappedV = (!mapped[a]) ? a : b;
    uint32_t mappedV = (mapped[a]) ? a : b;

    for (uint32_t i = 0, e = candidates.size(); i < e && remainingSolutions; ++i) {
        auto candPair = candidates[i];
        auto assign = GenAssignment(mPQubits, candPair.m, false);

        uint32_t remainingChildren = MaxChildren.getVal();
        if (isFirst) remainingChildren = MaxPartialSolutions.getVal();

        uint32_t maxMappingsAB = std::min(remainingChildren, remainingSolutions);
        std::vector<std::pair<uint32_t, uint32_t>> mappingsForAB;

        if (bothUnmapped) {
            // If both 'a' or 'b' are not mapped.
            for (uint32_t u = 0; u < mPQubits; ++u) {
                if (assign[u] != _undef) continue;
                for (uint32_t v : mArchGraph->adj(u)) {
                    if (assign[v] != _undef) continue;
                    mappingsForAB.push_back(std::make_pair(u, v));
                }
            }
        } else if (hasUnmapped) {
            // If only one of 'a' or 'b' are already mapped.
            uint32_t u = candPair.m[mappedV];
            for (uint32_t v : mArchGraph->adj(u)) {
                if (assign[v] == _undef) {
                    // This is one new candidate!
                    uint32_t from = (mappedV == a) ? u : v;
                    uint32_t to = (unmappedV == a) ? u : v;
                    mappingsForAB.push_back(std::make_pair(from, to));
                }
            }
        } else {
            // If both, 'a' and 'b' are already mapped.
            uint32_t u = candPair.m[a], v = candPair.m[b];

            if (mArchGraph->hasEdge(u, v) || mArchGraph->hasEdge(v, u))
                mappingsForAB.push_back(std::make_pair(u, v));
        }

        if (mappingsForAB.size() >= maxMappingsAB) {
            mappingsForAB.erase(mappingsForAB.begin() + maxMappingsAB, mappingsForAB.end());
        }

        for (auto& mappingCand : mappingsForAB) {
            CandPair nCand = candPair;
            nCand.m[a] = mappingCand.first;
            nCand.m[b] = mappingCand.second;

            if (!mArchGraph->hasEdge(mappingCand.first, mappingCand.second)) {
                nCand.cost += RevCost.getVal();
            }
            
            newCandidates.push_back(nCand);
        }

        remainingSolutions -= std::min(maxMappingsAB, (uint32_t) mappingsForAB.size());
    }

    mapped[a] = true;
    mapped[b] = true;

    return newCandidates;
}
