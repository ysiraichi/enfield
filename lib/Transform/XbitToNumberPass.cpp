#include "enfield/Transform/XbitToNumberPass.h"
#include "enfield/Analysis/NodeVisitor.h"
#include "enfield/Support/RTTI.h"
#include "enfield/Support/Defs.h"
#include "enfield/Support/uRefCast.h"

#include <cassert>
#include <algorithm>

// --------------------- XbitToNumber ------------------------
const efd::XbitToNumber::XbitMap&
efd::XbitToNumber::getQbitMap(NDGateDecl::Ref gate) const {
    if (gate != nullptr && lidQMap.find(gate) == lidQMap.end()) {
        ERR << "Trying to get an unknown gate information: `" << gate->getId()->getVal()
            << "`." << std::endl;
        ExitWith(ExitCode::EXIT_unknown_resource);
    }

    return (gate == nullptr) ? gidQMap : lidQMap.at(gate);
}

std::vector<uint32_t> efd::XbitToNumber::getRegUIds(std::string id) const {
    if (gidRegMap.find(id) == gidRegMap.end()) {
        ERR << "Register not found: `" << id << "`." << std::endl;
        ExitWith(ExitCode::EXIT_unknown_resource);
    }

    return gidRegMap.at(id);
}

uint32_t efd::XbitToNumber::getQUId(std::string id, NDGateDecl::Ref gate) const {
    auto& map = getQbitMap(gate);
    if (map.find(id) == map.end()) {
        std::string gateId = (gate == nullptr) ? "nullptr" : gate->getId()->getVal();
        ERR << "Qubit id not found inside gate (`" << gateId << "`): `" << id << "`." << std::endl;
        std::exit(12);
        // ExitWith(ExitCode::EXIT_unknown_resource);
    }
    return map.at(id).key;
}

uint32_t efd::XbitToNumber::getCUId(std::string id) const {
    if (gidCMap.find(id) == gidCMap.end()) {
        ERR << "Classical bit id not found: `" << id << "`." << std::endl;
        ExitWith(ExitCode::EXIT_unknown_resource);
    }
    return gidCMap.at(id).key;
}

uint32_t efd::XbitToNumber::getQSize(NDGateDecl::Ref gate) const {
    auto& map = getQbitMap(gate);
    return map.size();
}

uint32_t efd::XbitToNumber::getCSize() const {
    return gidCMap.size();
}

std::string efd::XbitToNumber::getQStrId(uint32_t id, NDGateDecl::Ref gate) const {
    auto& map = getQbitMap(gate);
    if (id >= map.size()) {
        ERR << "Id trying to access out of bounds value (of `" << map.size() << "`): `"
            << id << "`." << std::endl;
        ExitWith(ExitCode::EXIT_unknown_resource);
    }

    for (auto it = map.begin(), end = map.end(); it != end; ++it) {
        if (it->second.key == id) return it->first;
    }

    std::string gateId = (gate == nullptr) ? "nullptr" : gate->getId()->getVal();
    ERR << "UId not found inside gate (`" << gateId << "`): `" << id << "`." << std::endl;
    ExitWith(ExitCode::EXIT_unknown_resource);
}

std::string efd::XbitToNumber::getCStrId(uint32_t id) const {
    if (id >= gidCMap.size()) {
        ERR << "Classical bit id not found: `" << id << "`." << std::endl;
        ExitWith(ExitCode::EXIT_unknown_resource);
    }

    for (auto it = gidCMap.begin(), end = gidCMap.end(); it != end; ++it) {
        if (it->second.key == id) return it->first;
    }

    ERR << "UId not found: `" << id << "`." << std::endl;
    ExitWith(ExitCode::EXIT_unknown_resource);
}

efd::Node::Ref efd::XbitToNumber::getQNode(uint32_t id, NDGateDecl::Ref gate) const {
    auto str = getQStrId(id, gate);
    auto map = getQbitMap(gate);
    return map.at(str).node.get();
}

efd::Node::Ref efd::XbitToNumber::getCNode(uint32_t id) const {
    auto str = getCStrId(id);
    return gidCMap.at(str).node.get();
}

// --------------------- XbitToNumberWrapperPass ------------------------
uint8_t efd::XbitToNumberWrapperPass::ID = 0;

namespace efd {
    class XbitToNumberVisitor : public efd::NodeVisitor {
        private:
            efd::XbitToNumber& mXbitToNumber;

        public:
            XbitToNumberVisitor(efd::XbitToNumber& qtn) : mXbitToNumber(qtn) {}

            void visit(efd::NDRegDecl::Ref ref) override;
            void visit(efd::NDGateDecl::Ref ref) override;
    };
}

void efd::XbitToNumberVisitor::visit(NDRegDecl::Ref ref) {
    std::string id = ref->getId()->getVal();
    IntVal size = ref->getSize()->getVal();

    mXbitToNumber.gidRegMap[id] = std::vector<uint32_t>();

    auto mapref = &mXbitToNumber.gidQMap;
    if (ref->isCReg()) mapref = &mXbitToNumber.gidCMap;
    uint32_t basen = mapref->size();

    // For each register declaration, we associate a
    // number to each possible xbit.
    // 
    // For example, 'qreg q[5];' generates 'q[0]', 'q[1]', ...
    for (int i = 0; i < size.mV; ++i) {
        std::string key = id + "[" + std::to_string(i) +"]";
        auto ref = NDIdRef::Create(NDId::Create(id), NDInt::Create(std::to_string(i)));
        auto info = XbitToNumber::XbitInfo { basen + i, toShared(std::move(ref)) };
        mapref->insert(std::make_pair(key, info));
        mXbitToNumber.gidRegMap[id].push_back(basen + i);
    }
}

void efd::XbitToNumberVisitor::visit(NDGateDecl::Ref ref) {
    if (mXbitToNumber.lidQMap.find(ref) == mXbitToNumber.lidQMap.end()) {
        mXbitToNumber.lidQMap[ref] = efd::XbitToNumber::XbitMap();

        // Each quantum argument of each quantum gate declaration
        // will be mapped to a number.
        for (auto& childRef : *ref->getQArgs()) {
            NDId::Ref idref = dynCast<NDId>(childRef.get());

            XbitToNumber::XbitInfo info {
                (uint32_t) mXbitToNumber.lidQMap[ref].size(), toShared(idref->clone())
            };

            mXbitToNumber.lidQMap[ref][idref->getVal()] = info;
        }
    }
}

bool efd::XbitToNumberWrapperPass::run(QModule::Ref qmod) {
    mData.gidCMap.clear();
    mData.gidQMap.clear();
    mData.lidQMap.clear();

    XbitToNumberVisitor visitor(mData);

    for (auto it = qmod->reg_begin(), e = qmod->reg_end(); it != e; ++it) {
        (*it)->apply(&visitor);
    }

    for (auto it = qmod->gates_begin(), e = qmod->gates_end(); it != e; ++it) {
        (*it)->apply(&visitor);
    }

    return false;
}

efd::XbitToNumberWrapperPass::uRef efd::XbitToNumberWrapperPass::Create() {
    return uRef(new XbitToNumberWrapperPass());
}
