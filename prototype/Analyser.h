#ifndef ANALYSER_H
#define ANALYSER_H

#include <string>
#include <vector>

std::vector< std::pair<int, int> > readDependencies(std::string filename, int &qubits);

enum Method { ISO, DYN, NONE };

typedef std::vector<std::pair<int, int>> SwapVector;

struct MapResult {
    std::vector<int> initial;
    std::vector<SwapVector> swaps;
    int cost;
};

extern std::string PhysFilename;
extern std::string ProgFilename;

#endif
