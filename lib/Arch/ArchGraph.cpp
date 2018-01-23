#include "enfield/Arch/ArchGraph.h"
#include "enfield/Support/RTTI.h"

#include <cassert>
#include <fstream>
#include <sstream>

efd::ArchGraph::ArchGraph(uint32_t n, bool isGeneric) : Graph(K_ARCH, n), mNodes(n),
    mId(n, ""), mGeneric(isGeneric), mVID(0) {
}

efd::Node::Ref efd::ArchGraph::getNode(uint32_t i) const {
    assert(mNodes.size() > i && "Node index out of bounds.");
    return mNodes[i].get();
}

uint32_t efd::ArchGraph::putVertex(std::string s) {
    if (mStrToId.find(s) != mStrToId.end())
        return mStrToId[s];

    uint32_t id = mVID++;
    mId[id] = s;
    mStrToId[s] = id;
    return id;
}

uint32_t efd::ArchGraph::putVertex(Node::uRef node) {
    std::string s = node->toString();

    if (mStrToId.find(s) != mStrToId.end())
        return mStrToId[s];

    uint32_t id = putVertex(s);
    mNodes[id] = std::move(node);
    return id;
}

void efd::ArchGraph::putReg(std::string id, std::string size) {
    mRegs[id] = std::stoul(size);
}

uint32_t efd::ArchGraph::getUId(std::string s) {
    assert(hasSId(s) && "No such vertex with this string id.");
    return mStrToId[s];
}

bool efd::ArchGraph::hasSId(std::string s) const {
    return mStrToId.find(s) != mStrToId.end();
}

std::string efd::ArchGraph::getSId(uint32_t i) {
    assert(mId.size() > i && "Vertex index out of bounds.");
    return mId[i];
}

bool efd::ArchGraph::isReverseEdge(uint32_t i, uint32_t j) {
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

bool efd::ArchGraph::ClassOf(const Graph* g) {
    return g->isArch();
}

std::unique_ptr<efd::ArchGraph> efd::ArchGraph::Create(uint32_t n) {
    return std::unique_ptr<ArchGraph>(new ArchGraph(n));
}

static std::unique_ptr<efd::ArchGraph> ReadFromIn(std::istream& in) {
    uint32_t n;
    in >> n;

    std::unique_ptr<efd::ArchGraph> graph(efd::ArchGraph::Create(n));
    for (std::string uS, vS; in >> uS >> vS;) {
        uint32_t u = graph->putVertex(uS);
        uint32_t v = graph->putVertex(vS);
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
