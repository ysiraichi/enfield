#include "enfield/Support/Graph.h"

#include <iostream>
#include <fstream>
#include <sstream>

// ----------------------------- Graph -------------------------------
efd::Graph::Graph(Kind k, uint32_t n, Type ty) : mK(k), mN(n), mTy(ty) {
    mSuccessors.assign(n, std::set<uint32_t>());
    mPredecessors.assign(n, std::set<uint32_t>());
}

efd::Graph::Graph(uint32_t n, Type ty) : mK(K_GRAPH), mN(n), mTy(ty) {
    mSuccessors.assign(n, std::set<uint32_t>());
    mPredecessors.assign(n, std::set<uint32_t>());
}

std::string efd::Graph::vertexToString(uint32_t i) const {
    return std::to_string(i);
}

std::string efd::Graph::edgeToString(uint32_t i, uint32_t j, std::string op) const {
    return vertexToString(i) + " " + op + " " + vertexToString(j);
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

std::set<uint32_t> efd::Graph::adj(uint32_t i) const {
    std::set<uint32_t> adj;

    auto& succ = mSuccessors[i];
    auto& pred = mPredecessors[i];

    adj.insert(pred.begin(), pred.end());
    adj.insert(succ.begin(), succ.end());
    return adj;
}

bool efd::Graph::hasEdge(uint32_t i, uint32_t j) {
    std::set<uint32_t>& succ = this->succ(i);
    return succ.find(j) != succ.end();
}

void efd::Graph::putEdge(uint32_t i, uint32_t j) {
    mSuccessors[i].insert(j);
    mPredecessors[j].insert(i);

    if (!isDirectedGraph()) {
        mSuccessors[j].insert(i);
        mPredecessors[i].insert(j);
    }
}

bool efd::Graph::isWeighted() const {
    return mK == K_WEIGHTED;
}

bool efd::Graph::isArch() const {
    return mK == K_ARCH;
}

bool efd::Graph::isDirectedGraph() const {
    return mTy == Directed;
}

std::string efd::Graph::dotify(std::string name) const {
    bool isDirected = isDirectedGraph();
    std::string edgeOp, graphTy, dot;

    if (isDirected) { edgeOp = "->"; graphTy = "digraph"; }
    else { edgeOp = "--"; graphTy = "graph"; }

    dot = graphTy + " " + name + " {\n";
    for (uint32_t i = 0; i < mN; ++i) {
        dot += "    " + vertexToString(i) + ";\n";

        auto adjacent = mSuccessors[i];
        if (!isDirected) adjacent = adj(i);

        for (uint32_t j : adjacent) {
            if (isDirected || (!isDirected && j >= i))
                dot += "    " + edgeToString(i, j, edgeOp) + ";\n";
        }
    }
    dot += "}";
    return dot;
}

bool efd::Graph::ClassOf(const Graph* g) {
    return true;
}

efd::Graph::uRef efd::Graph::Create(uint32_t n, Type ty) {
    return std::unique_ptr<Graph>(new Graph(K_GRAPH, n, ty));
}

static efd::Graph::uRef ReadFromIn(std::istream& in, efd::Graph::Type ty) {
    uint32_t n;
    in >> n;

    std::unique_ptr<efd::Graph> graph(efd::Graph::Create(n, ty));
    for (uint32_t u, v; in >> u >> v;)
        graph->putEdge(u, v);

    return graph;
}

efd::Graph::uRef efd::Graph::Read(std::string filepath, Type ty) {
    std::ifstream in(filepath.c_str());
    return ReadFromIn(in, ty);
}

efd::Graph::uRef efd::Graph::ReadString(std::string graphStr, Type ty) {
    std::stringstream in(graphStr);
    return ReadFromIn(in, ty);
}
