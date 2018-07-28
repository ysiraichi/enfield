#include "enfield/Arch/ArchGraph.h"
#include "enfield/Analysis/Nodes.h"
#include "enfield/Support/RTTI.h"
#include "enfield/Support/Defs.h"

#include <fstream>
#include <sstream>

efd::ArchGraph::ArchGraph(uint32_t n, bool isGeneric)
    : Graph(K_ARCH, n, Directed), mNodes(n), mId(n, ""), mGeneric(isGeneric), mVID(0) {}

std::string efd::ArchGraph::vertexToString(uint32_t i) const {
    auto vstr = getNode(i)->toString(false);
    std::size_t pos;

    pos = 0;
    while ((pos = vstr.find(pos, '[')) != std::string::npos) {
        vstr = vstr.erase(pos, 1);
    }

    pos = 0;
    while ((pos = vstr.find(pos, ']')) != std::string::npos) {
        vstr = vstr.erase(pos, 1);
    }

    return vstr;
}

efd::Node::Ref efd::ArchGraph::getNode(uint32_t i) const {
    if (i >= mNodes.size()) {
        ERR << "Node index out of bounds (of `" << mNodes.size()
            << "`): `" << i << "`." << std::endl;
        ExitWith(ExitCode::EXIT_unreachable);
    }
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
    mRegs.push_back(std::make_pair(id, std::stoul(size)));
}

uint32_t efd::ArchGraph::getUId(std::string s) {
    if (!hasSId(s)) {
        ERR << "No such vertex with this string id: `" << s << "`." << std::endl;
        ExitWith(ExitCode::EXIT_unknown_resource);
    }
    return mStrToId[s];
}

bool efd::ArchGraph::hasSId(std::string s) const {
    return mStrToId.find(s) != mStrToId.end();
}

std::string efd::ArchGraph::getSId(uint32_t i) {
    if (i >= mId.size()) {
        ERR << "Vertex index out of bounds (of `" << mId.size() << "`): `"
            << i << "`." << std::endl;
        ExitWith(ExitCode::EXIT_unreachable);
    }
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
    uint32_t regs, qubits;
    in >> regs >> qubits;

    std::unique_ptr<efd::ArchGraph> graph(efd::ArchGraph::Create(qubits));

    for (uint32_t rid = 0; rid < regs; ++rid) {
        uint32_t regsize;
        std::string regname;
        in >> regname >> regsize;

        auto ndID = efd::NDId::Create(regname);
        graph->putReg(regname, std::to_string(regsize));
        for (uint32_t i = 0; i < regsize; ++i) {
            auto ndN = efd::NDInt::Create(std::to_string(i));
            auto ndIDCpy = efd::dynCast<efd::NDId>(ndID->clone().release());
            graph->putVertex(efd::NDIdRef::Create(efd::NDId::uRef(ndIDCpy), std::move(ndN)));
        }
    }

    for (std::string uS, vS; in >> uS >> vS;) {
        uint32_t u = graph->getUId(uS);
        uint32_t v = graph->getUId(vS);
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
