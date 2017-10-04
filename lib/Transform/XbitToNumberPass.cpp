#include "enfield/Transform/XbitToNumberPass.h"
#include "enfield/Analysis/NodeVisitor.h"
#include "enfield/Support/RTTI.h"
#include "enfield/Support/uRefCast.h"

#include <cassert>
#include <algorithm>

// --------------------- XbitToNumber ------------------------
const efd::XbitToNumber::XbitMap&
efd::XbitToNumber::getQbitMap(NDGateDecl::Ref gate) const {
    assert(gate == nullptr || lidQMap.find(gate) != lidQMap.end() &&
            "Trying to get an unknown gate information.");
    return (gate == nullptr) ? gidQMap : lidQMap.at(gate);
}

std::vector<unsigned> efd::XbitToNumber::getRegUIds(std::string id) const {
    assert(gidRegMap.find(id) != gidRegMap.end() && "Register not found.");
    return gidRegMap.at(id);
}

unsigned efd::XbitToNumber::getQUId(std::string id, NDGateDecl::Ref gate) const {
    auto& map = getQbitMap(gate);
    assert(map.find(id) != map.end() && "Qubit id not found.");
    return map.at(id).key;
}

unsigned efd::XbitToNumber::getCUId(std::string id) const {
    assert(gidCMap.find(id) != gidCMap.end() && "Classical bit id not found.");
    return gidCMap.at(id).key;
}

unsigned efd::XbitToNumber::getQSize(NDGateDecl::Ref gate) const {
    auto& map = getQbitMap(gate);
    return map.size();
}

unsigned efd::XbitToNumber::getCSize() const {
    return gidCMap.size();
}

std::string efd::XbitToNumber::getQStrId(unsigned id, NDGateDecl::Ref gate) const {
    auto& map = getQbitMap(gate);
    assert(id < map.size() && "Id trying to access out of bounds value.");

    for (auto it = map.begin(), end = map.end(); it != end; ++it) {
        if (it->second.key == id) return it->first;
    }

    assert(false && "UId not found.");
}

std::string efd::XbitToNumber::getCStrId(unsigned id) const {
    assert(id < gidCMap.size() && "Id trying to access out of bounds value.");

    for (auto it = gidCMap.begin(), end = gidCMap.end(); it != end; ++it) {
        if (it->second.key == id) return it->first;
    }

    assert(false && "UId not found.");
}

efd::Node::Ref efd::XbitToNumber::getQNode(unsigned id, NDGateDecl::Ref gate) const {
    auto str = getQStrId(id, gate);
    auto map = getQbitMap(gate);
    return map.at(str).node.get();
}

efd::Node::Ref efd::XbitToNumber::getCNode(unsigned id) const {
    auto str = getCStrId(id);
    return gidCMap.at(str).node.get();
}

// --------------------- XbitToNumberWrapperPass ------------------------
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

    mXbitToNumber.gidRegMap[id] = std::vector<unsigned>();

    auto mapref = &mXbitToNumber.gidQMap;
    if (ref->isCReg()) mapref = &mXbitToNumber.gidCMap;

    auto& map = *mapref;
    unsigned basen = map.size();

    // For each register declaration, we associate a
    // number to each possible xbit.
    // 
    // For example, 'qreg q[5];' generates 'q[0]', 'q[1]', ...
    for (int i = 0; i < size.mV; ++i) {
        std::string key = id + "[" + std::to_string(i) +"]";
        auto ref = NDIdRef::Create(NDId::Create(id), NDInt::Create(std::to_string(i)));
        map[key] = (XbitToNumber::XbitInfo { basen + i, toShared(std::move(ref)) });
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
            mXbitToNumber.lidQMap[ref][idref->getVal()] = (XbitToNumber::XbitInfo {
                    (unsigned) mXbitToNumber.lidQMap[ref].size(),
                    toShared(idref->clone()) });
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
