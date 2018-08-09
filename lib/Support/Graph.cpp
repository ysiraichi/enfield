#include "enfield/Support/Graph.h"

#include <iostream>
#include <fstream>
#include <sstream>

using namespace efd;

// ----------------------------- Static -------------------------------
static const std::string _VerticesLabel_ = JsonFields<Graph>::_VerticesLabel_;
static const std::string _AdjListLabel_ = JsonFields<Graph>::_AdjListLabel_;
static const std::string _TypeLabel_ = JsonFields<Graph>::_TypeLabel_;
static const std::string _VLabel_ = JsonFields<Graph>::_VLabel_;

static Graph::uRef ReadFromIn(std::istream& in, Graph::Type ty) {
    auto root = JsonInputParser<Graph>::Parse(in);
    return FromJsonGetter<Graph>::Get(root);
}

// ----------------------------- Graph -------------------------------
Graph::Graph(Kind k, uint32_t n, Type ty) : mK(k), mN(n), mTy(ty) {
    mSuccessors.assign(n, std::set<uint32_t>());
    mPredecessors.assign(n, std::set<uint32_t>());
}

Graph::Graph(uint32_t n, Type ty) : mK(K_GRAPH), mN(n), mTy(ty) {
    mSuccessors.assign(n, std::set<uint32_t>());
    mPredecessors.assign(n, std::set<uint32_t>());
}

std::string Graph::vertexToString(uint32_t i) const {
    return std::to_string(i);
}

std::string Graph::edgeToString(uint32_t i, uint32_t j, std::string op) const {
    return vertexToString(i) + " " + op + " " + vertexToString(j);
}

uint32_t Graph::inDegree(uint32_t i) const {
    return mPredecessors[i].size();
}

uint32_t Graph::outDegree(uint32_t i) const {
    return mSuccessors[i].size();
}

uint32_t Graph::size() const {
    return mN;
}

std::set<uint32_t>& Graph::succ(uint32_t i) {
    return mSuccessors[i];
}

std::set<uint32_t>& Graph::pred(uint32_t i) {
    return mPredecessors[i];
}

std::set<uint32_t> Graph::adj(uint32_t i) const {
    std::set<uint32_t> adj;

    auto& succ = mSuccessors[i];
    auto& pred = mPredecessors[i];

    adj.insert(pred.begin(), pred.end());
    adj.insert(succ.begin(), succ.end());
    return adj;
}

bool Graph::hasEdge(uint32_t i, uint32_t j) {
    std::set<uint32_t>& succ = this->succ(i);
    return succ.find(j) != succ.end();
}

void Graph::putEdge(uint32_t i, uint32_t j) {
    mSuccessors[i].insert(j);
    mPredecessors[j].insert(i);

    if (!isDirectedGraph()) {
        mSuccessors[j].insert(i);
        mPredecessors[i].insert(j);
    }
}

bool Graph::isWeighted() const {
    return mK == K_WEIGHTED;
}

bool Graph::isArch() const {
    return mK == K_ARCH;
}

bool Graph::isDirectedGraph() const {
    return mTy == Directed;
}

std::string Graph::dotify(std::string name) const {
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

bool Graph::ClassOf(const Graph* g) {
    return true;
}

Graph::uRef Graph::Create(uint32_t n, Type ty) {
    return std::unique_ptr<Graph>(new Graph(K_GRAPH, n, ty));
}

Graph::uRef Graph::Read(std::string filepath, Type ty) {
    std::ifstream in(filepath.c_str());
    return ReadFromIn(in, ty);
}

Graph::uRef Graph::ReadString(std::string graphStr, Type ty) {
    std::stringstream in(graphStr);
    return ReadFromIn(in, ty);
}

// ----------------------------- JsonFields -------------------------------
const std::string JsonFields<Graph>::_VerticesLabel_ = "vertices";
const std::string JsonFields<Graph>::_AdjListLabel_ = "adj";
const std::string JsonFields<Graph>::_TypeLabel_ = "type";
const std::string JsonFields<Graph>::_VLabel_ = "v";

// ----------------------------- JsonInputParser -------------------------------
Json::Value JsonInputParser<Graph>::Parse(std::istream& in) {
    Json::Value root;
    in >> root;

    if (!root[_VerticesLabel_].isInt()) {
        ERR << "Graph parsing error: field `" << _VerticesLabel_
            << "` not found or not an int." << std::endl;
        ExitWith(ExitCode::EXIT_json_parsing_error);
    }

    if (!root[_AdjListLabel_].isArray()) {
        ERR << "Graph parsing error: field `" << _AdjListLabel_
            << "` not found or not an array." << std::endl;
        ExitWith(ExitCode::EXIT_json_parsing_error);
    }

    if (!root[_TypeLabel_].isString()) {
        ERR << "Graph parsing error: field `" << _TypeLabel_
            << "` not found or not a string." << std::endl;
        ExitWith(ExitCode::EXIT_json_parsing_error);
    }

    auto type = root[_TypeLabel_].asString();
    if (type != "D" && type != "U") {
        ERR << "Graph parsing error: field `" << _TypeLabel_
            << "` should be `U` (undirected) or `D` (directed)." << std::endl;
        ExitWith(ExitCode::EXIT_json_parsing_error);
    }

    for (uint32_t i = 0, e = root[_AdjListLabel_].size(); i < e; ++i) {
        auto &iList = root[_AdjListLabel_][i];

        if (!iList.isArray()) {
            ERR << "Graph parsing error: `" << _AdjListLabel_
                << "`[" << i << "] is not an array." << std::endl;
            ExitWith(ExitCode::EXIT_json_parsing_error);
        }

        for (uint32_t j = 0, f = root[_AdjListLabel_][i].size(); j < f; ++j) {
            auto &jElem = iList[j];

            if (!jElem.isObject()) {
                ERR << "Graph parsing error: `" << _AdjListLabel_
                    << "`[" << i << "][" << j << "] is not an object." << std::endl;
                ExitWith(ExitCode::EXIT_json_parsing_error);
            }
        }
    }
}

// ----------------------------- FromJsonGetter -------------------------------
std::unique_ptr<Graph> FromJsonGetter<Graph>::Get(const Json::Value& root) {
    auto vertices = root[_VerticesLabel_].asUInt();
    auto tyString = root[_TypeLabel_].asString();
    Graph::Type ty;

    if (tyString == "D") ty = Graph::Type::Directed;
    else ty = Graph::Type::Undirected;

    auto graph = Graph::Create(vertices, ty);

    for (uint32_t i = 0, e = root[_AdjListLabel_].size(); i < e; ++i) {
        auto &iList = root[_AdjListLabel_][i];
        
        for (uint32_t j = 0, f = iList.size(); j < f; ++j) {
            auto &jElem = iList[j];

            if (!jElem[_VLabel_].isUInt()) {
                ERR << "Graph parsing error: `" << _AdjListLabel_
                    << "`[" << i << "][" << j << "][`" << _VLabel_ << "`] is not an uint."
                    << std::endl;
                ExitWith(ExitCode::EXIT_json_parsing_error);
            }

            graph->putEdge(i, jElem[_VLabel_].asUInt());
        }
    }
}
