#include "enfield/Arch/ArchGraph.h"
#include "enfield/Support/RTTI.h"

#include <cassert>
#include <fstream>
#include <sstream>

efd::ArchGraph::ArchGraph(unsigned n, bool isGeneric) : Graph(n), mNodes(n),
    mVID(0), mGeneric(isGeneric), mId(n, "") {
}

efd::Node::Ref efd::ArchGraph::getNode(unsigned i) const {
    assert(mNodes.size() > i && "Node index out of bounds.");
    return mNodes[i].get();
}

unsigned efd::ArchGraph::putVertex(std::string s) {
    if (mStrToId.find(s) != mStrToId.end())
        return mStrToId[s];

    unsigned id = mVID++;
    mId[id] = s;
    mStrToId[s] = id;
    return id;
}

unsigned efd::ArchGraph::putVertex(Node::uRef node) {
    std::string s = node->toString();

    if (mStrToId.find(s) != mStrToId.end())
        return mStrToId[s];

    unsigned id = putVertex(s);
    mNodes[id] = std::move(node);
    return id;
}

void efd::ArchGraph::putReg(std::string id, std::string size) {
    mRegs[id] = std::stoul(size);
}

unsigned efd::ArchGraph::getUId(std::string s) {
    assert(mStrToId.find(s) != mStrToId.end() &&
            "No such vertex with this string id.");
    return mStrToId[s];
}

std::string efd::ArchGraph::getSId(unsigned i) {
    assert(mId.size() > i && "Vertex index out of bounds.");
    return mId[i];
}

bool efd::ArchGraph::isReverseEdge(unsigned i, unsigned j) {
    auto& succ = mSuccessors[i];
    auto& pred = mPredecessors[i];
    return succ.find(j) == succ.end() && pred.find(j) != pred.end();
}

bool efd::ArchGraph::isGeneric() {
    return mGeneric;
}

efd::ArchGraph::RegsIterator efd::ArchGraph::reg_begin() {
    return mRegs.begin();
}

efd::ArchGraph::RegsIterator efd::ArchGraph::reg_end() {
    return mRegs.end();
}

std::unique_ptr<efd::ArchGraph> efd::ArchGraph::Create(unsigned n) {
    return std::unique_ptr<ArchGraph>(new ArchGraph(n));
}

static std::unique_ptr<efd::ArchGraph> ReadFromIn(std::istream& in) {
    unsigned n;
    in >> n;

    std::unique_ptr<efd::ArchGraph> graph(efd::ArchGraph::Create(n));
    for (std::string uS, vS; in >> uS >> vS;) {
        unsigned u = graph->putVertex(uS);
        unsigned v = graph->putVertex(vS);
        graph->putEdge(u, v);
    }

    return graph;
}

std::unique_ptr<efd::ArchGraph> efd::ArchGraph::Read(std::string filepath) {
    std::ifstream in(filepath.c_str());
    return ReadFromIn(in);
}

std::unique_ptr<efd::ArchGraph> efd::ArchGraph::ReadString(std::string graphStr) {
    std::stringstream in(graphStr);
    return ReadFromIn(in);
}
