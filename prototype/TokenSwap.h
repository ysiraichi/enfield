#ifndef TOKEN_SWAP_H
#define TOKEN_SWAP_H

#include "Graph.h"
#include <unordered_map>

struct PermVal {
    int idx;
    std::vector<int> perm;
};

std::string vecToKey(std::vector<int>& v);
void genPermutationMap(int phys, std::unordered_map<std::string, PermVal>& permMap);

int getNofSwaps(std::vector<int>& source, std::vector<int>& target);
std::vector<std::pair<int, int>> getSwaps(std::vector<int>& source, std::vector<int>& target);
void computeSwaps(Graph& archG);

#endif
