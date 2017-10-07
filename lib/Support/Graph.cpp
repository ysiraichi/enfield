#include "enfield/Support/Graph.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <cassert>

// ----------------------------- Graph -------------------------------
efd::Graph::Graph(Kind k, uint32_t n) : mK(k), mN(n) {
    mSuccessors.assign(n, std::set<uint32_t>());
    mPredecessors.assign(n, std::set<uint32_t>());
}

uint32_t efd::Graph::inDegree(uint32_t i) const {
    return mPredecessors[i].size();
}

uint32_t efd::Graph::outDegree(uint32_t i) const {
    return mSuccessors[i].size();
}

uint32_t efd::Graph::size() const {
    return mN;
}

std::set<uint32_t>& efd::Graph::succ(uint32_t i) {
    return mSuccessors[i];
}

std::set<uint32_t>& efd::Graph::pred(uint32_t i) {
    return mPredecessors[i];
}

bool efd::Graph::hasEdge(uint32_t i, uint32_t j) {
    std::set<uint32_t>& succ = this->succ(i);
    return succ.find(j) != succ.end();
}

void efd::Graph::putEdge(uint32_t i, uint32_t j) {
    mSuccessors[i].insert(j);
    mPredecessors[j].insert(i);
}

bool efd::Graph::isWeighted() const {
    return mK == K_WEIGHTED;
}

bool efd::Graph::isArch() const {
    return mK == K_ARCH;
}

bool efd::Graph::ClassOf(const Graph* g) {
    return true;
}

efd::Graph::uRef efd::Graph::Create(uint32_t n) {
    return std::unique_ptr<Graph>(new Graph(K_GRAPH, n));
}

static efd::Graph::uRef ReadFromIn(std::istream& in) {
    uint32_t n;
    in >> n;

    std::unique_ptr<efd::Graph> graph(efd::Graph::Create(n));
    for (uint32_t u, v; in >> u >> v;)
        graph->putEdge(u, v);

    return graph;
}

efd::Graph::uRef efd::Graph::Read(std::string filepath) {
    std::ifstream in(filepath.c_str());
    return ReadFromIn(in);
}

efd::Graph::uRef efd::Graph::ReadString(std::string graphStr) {
    std::stringstream in(graphStr);
    return ReadFromIn(in);
}
