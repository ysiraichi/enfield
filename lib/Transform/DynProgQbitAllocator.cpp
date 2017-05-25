#include "enfield/Transform/DynProgQbitAllocator.h"

#include <unordered_map>
#include <limits>
#include <iostream>
#include <algorithm>

const unsigned SWAP_COST = 7;
const unsigned REV_COST = 4;
const unsigned UNREACH = std::numeric_limits<unsigned>::max();

static unsigned min(unsigned a, unsigned b) {
    bool isAUnreach = (a == UNREACH);
    bool isBUnreach = (b == UNREACH);

    if (isAUnreach && isBUnreach)
        return UNREACH;

    if (isAUnreach) return b;
    if (isBUnreach) return a;
    return std::min(a, b);
}

struct PermVal {
    unsigned idx;
    std::vector<unsigned> perm;
};

struct SwapVal {
    unsigned root;
    unsigned cost;
};

static std::string vecToKey(std::vector<unsigned> &v) {
    std::string key = "";
    for (unsigned x : v)
        key += std::to_string(x) + "|";
    return key;
}

static std::unordered_map<std::string, PermVal> genPermutationMap(int phys, int prog) {
    std::unordered_map<std::string, PermVal> permMap;

    std::vector<bool> selector(phys-prog, false);
    while (selector.size() != phys)
        selector.push_back(true);

    unsigned idx = 0;
    do {

        std::vector<unsigned> perm;
        for (unsigned i = 0; i < phys; ++i)
            if (selector[i])
                perm.push_back(i);

        do {
            permMap.insert(std::pair<std::string, PermVal>(vecToKey(perm), { idx++, perm }));
        } while (std::next_permutation(perm.begin(), perm.end()));

    } while (next_permutation(selector.begin(), selector.end()));

    return permMap;
}

static unsigned getNQbits(efd::QbitAllocator::DepsSet& deps) {
    std::set<unsigned> qbitSet;
    for (auto& pDeps : deps) {
        for (auto& dep : pDeps) {
            qbitSet.insert(dep.mFrom);
            qbitSet.insert(dep.mTo);
        }
    }
    return qbitSet.size();
}

static std::vector<unsigned> genAssign(std::vector<unsigned> mapping) {
    std::vector<unsigned> assign(mapping.size(), -1);
    for (int i = 0, e = mapping.size(); i < e; ++i)
        assign[mapping[i]] = i;
    return assign;
}

static void swapAll(std::vector<unsigned>& uMap, 
        std::vector<efd::QbitAllocator::Swap> swapSet) {
    std::vector<unsigned> assign = genAssign(uMap);

    for (auto swp : swapSet) {
        unsigned u = swp.mU;
        unsigned v = swp.mV;

        unsigned progU = assign[u];
        unsigned progV = assign[v];

        std::swap(uMap[progU], uMap[progV]);
        std::swap(assign[u], assign[v]);
    }
}

efd::DynProgQbitAllocator::DynProgQbitAllocator
(QModule* qmod, Graph* pGraph, SwapFinder* sFind, DependencyBuilderPass* depPass) 
    : QbitAllocator(qmod, pGraph, sFind, depPass) {
}

static void printSV(unsigned idx, std::vector<unsigned> map, SwapVal& sv, bool endl) {
    std::cout << "{id:" << idx << ", [" << map[0];
    for (unsigned i = 1; i < map.size(); ++i)
        std::cout << ", " << map[i];
    std::cout << "], root:" << sv.root << ", cost:" << sv.cost << "}";
    if (endl) std::cout << std::endl;
}

std::vector<unsigned> efd::DynProgQbitAllocator::getUMapping(DepsSet& deps) {
    std::unordered_map<std::string, PermVal> permMap = genPermutationMap
        (mArchGraph->size(), getNumQbits());

    int permN = permMap.size();
    int depN = deps.size();

    std::vector<std::vector<unsigned>*> permIdMap(permN, nullptr);
    for (auto &pair : permMap)
        permIdMap[pair.second.idx] = &pair.second.perm;

    // Map with the minimum number of swaps at time 'i'.
    std::vector<std::vector<SwapVal>> swaps(permN, std::vector<SwapVal>
            (depN + 1, SwapVal { UNREACH, std::numeric_limits<unsigned>::max() }));

    for (unsigned i = 0; i < permN; ++i)
        swaps[i][0] = { i, 0 };

    unsigned t = 0;
    for (auto it = deps.begin(); it != deps.end(); ++it) {
        ++t;

        while (it->getSize() > 1) {
            it = inlineDep(it);
            int newDepN = deps.size();

            for (int i = 0; i < permN; ++i)
                for (int j = depN + 1; j <= newDepN; ++j)
                    swaps[i].push_back({ UNREACH, std::numeric_limits<unsigned>::max() });

            depN = newDepN;
        }

        // Always get the first parallel dep.
        Dep dep = (*it)[0];

        /*
        std::cout << "==----------------------------------------------==" << std::endl;
        std::cout << "Dep: " << dep.mFrom << " -> " << dep.mTo << std::endl;
        std::cout << "t: " << t << std::endl;
        */

        for (auto pair : permMap) {
            unsigned idx = pair.second.idx;
            std::vector<unsigned> mapping = pair.second.perm;
            
            unsigned sourceIdx = idx;
            unsigned targetIdx = sourceIdx;

            SwapVal *source = &swaps[sourceIdx][t-1];
            SwapVal *target = &swaps[targetIdx][t];

            if (source->root == UNREACH)
                continue;

            SwapVal finalVal;
            if (min(source->cost, target->cost) == source->cost) {
                finalVal = *source;
            } else {
                finalVal = *target;
            }

            unsigned u = mapping[dep.mTo];
            unsigned v = mapping[dep.mFrom];
            if (!mArchGraph->hasEdge(u, v)) {
                std::vector<unsigned> newMapping = mapping;

                std::vector<Swap> swapSet = mSFind->findSwaps
                    (RestrictionVector { Rest { u, v } });
                swapAll(newMapping, swapSet);

                targetIdx = permMap[vecToKey(newMapping)].idx;
                target = &swaps[targetIdx][t];

                unsigned newCalcCost = source->cost + (swapSet.size() * SWAP_COST);
                unsigned oldCalcCost = target->cost;

                if (min(newCalcCost, oldCalcCost) == newCalcCost)
                    finalVal = { source->root, newCalcCost };
                else
                    finalVal = *target;

                mapping = newMapping;
            }

            u = mapping[dep.mFrom];
            v = mapping[dep.mTo];
            if (mArchGraph->isReverseEdge(u, v))
                finalVal.cost += REV_COST;

            /*
            printSV(sourceIdx, pair.second.perm, *source, false);
            std::cout << " -> ";
            printSV(1, mapping, finalVal, true);
            */

            *target = finalVal;
        }
    }

    SwapVal minCost = swaps[0][depN];
    for (unsigned i = 1; i < permN; ++i) {
        unsigned minVal = min(minCost.cost, swaps[i][depN].cost);
        minCost = (minVal == minCost.cost) ? minCost : swaps[i][depN];
    }
    return *(permIdMap[minCost.root]);
}

efd::QbitAllocator::Mapping efd::DynProgQbitAllocator::solveDependencies(DepsSet& deps) {
    // Map Prog -> Arch
    std::vector<unsigned> uMap = getUMapping(deps);
    // Map Arch -> Prog
    std::vector<unsigned> uAssignMap = genAssign(uMap);

    // Adding the swaps for this mapping.
    for (auto it = deps.begin(), e = deps.end(); it != e; ++it) {
        Dep dep = (*it)[0];
        NodeRef call = it->mCallPoint;

        unsigned progU = dep.mFrom;
        unsigned progV = dep.mTo;

        unsigned archU = uMap[progU];
        unsigned archV = uMap[progV];

        if (!mArchGraph->hasEdge(archU, archV)) {
            RestrictionVector restV { Rest { archU, archV }};
            SwapVector swapV = mSFind->findSwaps(restV);

            for (auto swp : swapV) {
                // (u, v) is an edge in the Program.
                unsigned u = uAssignMap[swp.mU]; 
                unsigned v = uAssignMap[swp.mV]; 
                insertSwapBefore(*it, u, v);
                std::swap(uMap[u], uMap[v]);
                std::swap(uAssignMap[swp.mU], uAssignMap[swp.mV]);
            }
        }
    }

    return uMap;
}

efd::DynProgQbitAllocator* efd::DynProgQbitAllocator::Create
(QModule* qmod, Graph* pGraph, SwapFinder* sFind, DependencyBuilderPass* depPass) {

    return new DynProgQbitAllocator(qmod, pGraph, sFind, depPass);
}
