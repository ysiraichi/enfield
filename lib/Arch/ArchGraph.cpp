#include "enfield/Arch/ArchGraph.h"
#include "enfield/Analysis/Nodes.h"
#include "enfield/Support/RTTI.h"
#include "enfield/Support/Defs.h"
#include "enfield/Support/uRefCast.h"

#include <fstream>
#include <sstream>

using namespace efd;

ArchGraph::ArchGraph(uint32_t n, bool isGeneric)
    : WeightedGraph<double>(K_ARCH, n, Directed),
      mNodes(n),
      mId(n, ""),
      mGeneric(isGeneric),
      mVID(0) {}

std::string ArchGraph::vertexToString(uint32_t i) const {
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

Node::Ref ArchGraph::getNode(uint32_t i) const {
    EfdAbortIf(i >= mNodes.size(),
               "Node index out of bounds (of `" << mNodes.size() << "`): `" << i << "`.");
    return mNodes[i].get();
}

uint32_t ArchGraph::putVertex(std::string s) {
    if (mStrToId.find(s) != mStrToId.end())
        return mStrToId[s];

    uint32_t id = mVID++;
    mId[id] = s;
    mStrToId[s] = id;
    return id;
}

uint32_t ArchGraph::putVertex(Node::uRef node) {
    std::string s = node->toString();

    if (mStrToId.find(s) != mStrToId.end())
        return mStrToId[s];

    uint32_t id = putVertex(s);
    mNodes[id] = std::move(node);
    return id;
}

void ArchGraph::putReg(std::string id, std::string size) {
    mRegs.push_back(std::make_pair(id, std::stoul(size)));
}

uint32_t ArchGraph::getUId(std::string s) {
    EfdAbortIf(!hasSId(s), "No such vertex with this string id: `" << s << "`.");
    return mStrToId[s];
}

bool ArchGraph::hasSId(std::string s) const {
    return mStrToId.find(s) != mStrToId.end();
}

std::string ArchGraph::getSId(uint32_t i) {
    EfdAbortIf(i >= mId.size(),
               "Vertex index out of bounds (of `" << mId.size() << "`): `" << i << "`.");
    return mId[i];
}

bool ArchGraph::isGeneric() {
    return mGeneric;
}

ArchGraph::RegsIterator ArchGraph::reg_begin() {
    return mRegs.begin();
}

ArchGraph::RegsIterator ArchGraph::reg_end() {
    return mRegs.end();
}

bool ArchGraph::ClassOf(const Graph* g) {
    return g->isArch();
}

std::unique_ptr<ArchGraph> ArchGraph::Create(uint32_t n) {
    return std::unique_ptr<ArchGraph>(new ArchGraph(n));
}

// ----------------------------- JsonFields -------------------------------
const std::string JsonFields<ArchGraph>::_QubitsLabel_ = "qubits";
const std::string JsonFields<ArchGraph>::_RegistersLabel_ = "registers";
const std::string JsonFields<ArchGraph>::_NameLabel_ = "name";
const std::string JsonFields<ArchGraph>::_AdjListLabel_ = "adj";
const std::string JsonFields<ArchGraph>::_VLabel_ = "v";
const std::string JsonFields<ArchGraph>::_WeightLabel_ = "w";
const std::string JsonFields<ArchGraph>::_RegLabel_ = "reg";
const std::string JsonFields<ArchGraph>::_IdLabel_ = "id";

// ----------------------------- JsonBackendParser -------------------------------
std::unique_ptr<ArchGraph> JsonBackendParser<ArchGraph>::Parse(const Json::Value& root) {
    typedef Json::ValueType Ty;
    const std::string _ArchGraphErrorPrefix_ = "ArchGraph parsing error";

    JsonCheckTypeError(_ArchGraphErrorPrefix_, root,
                       JsonFields<ArchGraph>::_QubitsLabel_,
                       { Ty::intValue, Ty::uintValue });
    JsonCheckTypeError(_ArchGraphErrorPrefix_, root,
                       JsonFields<ArchGraph>::_AdjListLabel_,
                       { Ty::arrayValue });
    JsonCheckTypeError(_ArchGraphErrorPrefix_, root,
                       JsonFields<ArchGraph>::_RegistersLabel_,
                       { Ty::arrayValue });

    auto qubits = root[JsonFields<ArchGraph>::_QubitsLabel_].asUInt();
    auto graph = ArchGraph::Create(qubits);

    uint32_t totalQubits = 0;
    auto &registers = root[JsonFields<ArchGraph>::_RegistersLabel_];
    for (uint32_t i = 0, e = registers.size(); i < e; ++i) {
        JsonCheckTypeError(_ArchGraphErrorPrefix_, registers, i, { Ty::objectValue });

        auto &reg = registers[i];
        JsonCheckTypeError(_ArchGraphErrorPrefix_, reg,
                           JsonFields<ArchGraph>::_NameLabel_,
                           { Ty::stringValue });
        JsonCheckTypeError(_ArchGraphErrorPrefix_, reg,
                           JsonFields<ArchGraph>::_QubitsLabel_,
                           { Ty::intValue, Ty::uintValue });
        auto name = reg[JsonFields<ArchGraph>::_NameLabel_].asString();
        auto qubits = reg[JsonFields<ArchGraph>::_QubitsLabel_].asUInt();
        graph->putReg(name, std::to_string(qubits));

        totalQubits += qubits;

        auto idNode = NDId::Create(name);
        for (uint32_t j = 0; j < qubits; ++j) {
            auto clone = uniqueCastForward<NDId>(idNode->clone());
            graph->putVertex(NDIdRef::Create(NDId::uRef(std::move(clone)),
                                             NDInt::Create(std::to_string(j))));
        }
    }

    EfdAbortIf(totalQubits != qubits,
               "Sum of qubits doesn't match total provided. Sum: " << totalQubits
               << " != Total: " << qubits);
    
    auto &adj = root[JsonFields<ArchGraph>::_AdjListLabel_];
    for (uint32_t i = 0; i < qubits; ++i) {
        JsonCheckTypeError(_ArchGraphErrorPrefix_, adj, i, { Ty::arrayValue });
        auto &iList = adj[i];

        for (uint32_t j = 0, f = iList.size(); j < f; ++j) {
            JsonCheckTypeError(_ArchGraphErrorPrefix_, iList, j, { Ty::objectValue });

            auto &jElem = iList[j];
            JsonCheckTypeError(_ArchGraphErrorPrefix_, jElem,
                               JsonFields<ArchGraph>::_VLabel_,
                               { Ty::stringValue });

            auto vString = jElem[JsonFields<ArchGraph>::_VLabel_].asString();
            auto v = graph->getUId(vString);
            graph->putEdge(i, v, 1);

            if (jElem.isMember(JsonFields<ArchGraph>::_WeightLabel_)) {
                JsonCheckTypeError(_ArchGraphErrorPrefix_, jElem,
                                   JsonFields<ArchGraph>::_WeightLabel_,
                                   { Ty::intValue, Ty::uintValue, Ty::realValue });
                auto w = jElem[JsonFields<ArchGraph>::_WeightLabel_].asDouble();
                // In the Json, the standard is to have the probability of error.
                // What we want is the probability of succes.
                graph->setW(i, v, 1 - w);
            }
        }
    }

    return graph;
}
