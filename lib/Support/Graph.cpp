#include "enfield/Support/Graph.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <cassert>

// ----------------------------- Graph -------------------------------
efd::Graph::Graph(unsigned n) : mN(n) {
    mSuccessors.assign(n, std::set<unsigned>());
    mPredecessors.assign(n, std::set<unsigned>());
}

unsigned efd::Graph::inDegree(unsigned i) const {
    return mPredecessors[i].size();
}

unsigned efd::Graph::outDegree(unsigned i) const {
    return mSuccessors[i].size();
}

unsigned efd::Graph::size() const {
    return mN;
}

std::set<unsigned>& efd::Graph::succ(unsigned i) {
    return mSuccessors[i];
}

std::set<unsigned>& efd::Graph::pred(unsigned i) {
    return mPredecessors[i];
}

bool efd::Graph::hasEdge(unsigned i, unsigned j) {
    std::set<unsigned>& succ = this->succ(i);
    return succ.find(j) != succ.end();
}

void efd::Graph::putEdge(unsigned i, unsigned j) {
    mSuccessors[i].insert(j);
    mPredecessors[j].insert(i);
}

std::unique_ptr<efd::Graph> efd::Graph::Create(unsigned n) {
    return std::unique_ptr<Graph>(new Graph(n));
}

static std::unique_ptr<efd::Graph> ReadFromIn(std::istream& in) {
    unsigned n;
    in >> n;

    std::unique_ptr<efd::Graph> graph(efd::Graph::Create(n));
    for (unsigned u, v; in >> u >> v;)
        graph->putEdge(u, v);

    return graph;
}

std::unique_ptr<efd::Graph> efd::Graph::Read(std::string filepath) {
    std::ifstream in(filepath.c_str());
    return ReadFromIn(in);
}

std::unique_ptr<efd::Graph> efd::Graph::ReadString(std::string graphStr) {
    std::stringstream in(graphStr);
    return ReadFromIn(in);
}
