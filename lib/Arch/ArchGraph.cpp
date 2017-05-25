#include "enfield/Arch/ArchGraph.h"
#include "enfield/Support/RTTI.h"

#include <cassert>
#include <fstream>
#include <sstream>

efd::ArchGraph::ArchGraph(unsigned n) : Graph(n), mNodes(n, nullptr) {
}

void efd::ArchGraph::preprocessVertexString(unsigned i, std::string s) {
    if (mNodes[i] != nullptr) return;

    std::size_t idEndPos = s.find("[");
    std::size_t nEndPos = s.find("]");
    assert(idEndPos != std::string::npos && nEndPos != std::string::npos &&
            "Vertex string not a register reference.");

    std::string id = s.substr(0, idEndPos);
    std::string nStr = s.substr(idEndPos+1, nEndPos - idEndPos - 1);

    NodeRef refId = NDId::Create(id);
    NodeRef refInt = NDInt::Create(nStr);

    if (mRegs.find(id) == mRegs.end())
        mRegs[id] = dynCast<NDId>(refId);
    mNodes[mGID] = NDIdRef::Create(refId, refInt);
}

efd::NodeRef efd::ArchGraph::getNode(unsigned i) const {
    assert(mNodes.size() > i && "Node index out of bounds.");
    return mNodes[i];
}

unsigned efd::ArchGraph::putVertex(std::string s) {
    unsigned curId = Graph::putVertex(s);
    preprocessVertexString(curId, s);
    return curId;
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

    graph->buildReverseGraph();
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
