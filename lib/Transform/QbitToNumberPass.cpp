#include "enfield/Transform/QbitToNumberPass.h"
#include "enfield/Analysis/NodeVisitor.h"
#include "enfield/Support/RTTI.h"
#include "enfield/Support/uRefCast.h"

#include <cassert>
#include <algorithm>

// --------------------- QbitToNumber ------------------------
efd::QbitToNumber::QbitToNumber() {
}

const efd::QbitToNumber::QbitMap* efd::QbitToNumber::getMap(NDGateDecl* gate) const {
    const QbitMap* map = &mGIdMap;

    if (gate != nullptr) {
        assert(mLIdMap.find(gate) != mLIdMap.end() && \
                "Gate not parsed.");
        map = &mLIdMap.at(gate);
    }

    return map;
}

unsigned efd::QbitToNumber::getUId(std::string id, NDGateDecl::Ref gate) const {
    const QbitMap* map = getMap(gate);

    unsigned index = 0;
    auto it = map->begin();
    for (auto end = map->end(); it != end; ++it, ++index)
        if (it->mKey == id) break;

    assert(it != map->end() && "Id not found.");
    return index;
}

unsigned efd::QbitToNumber::getSize(NDGateDecl::Ref gate) const {
    const QbitMap* map = getMap(gate);
    return map->size();
}

std::string efd::QbitToNumber::getStrId(unsigned id, NDGateDecl::Ref gate) const {
    const QbitMap* map = getMap(gate);
    assert(id < map->size() && "Id trying to access out of bounds value.");
    return map->at(id).mKey;
}

efd::Node::Ref efd::QbitToNumber::getNode(unsigned id, NDGateDecl::Ref gate) const {
    const QbitMap* map = getMap(gate);
    assert(id < map->size() && "Id trying to access out of bounds value.");
    return map->at(id).mRef.get();
}

// --------------------- QbitToNumberWrapperPass ------------------------
namespace efd {
    class QbitToNumberVisitor : public efd::NodeVisitor {
        private:
            efd::QbitToNumber& mQbitToNumber;

        public:
            QbitToNumberVisitor(efd::QbitToNumber& qtn) : mQbitToNumber(qtn) {}

            void visit(efd::NDRegDecl::Ref ref) override;
            void visit(efd::NDGateDecl::Ref ref) override;
    };
}

void efd::QbitToNumberVisitor::visit(NDRegDecl::Ref ref) {
    if (ref->isCReg()) return;

    std::string id = ref->getId()->getVal();
    IntVal size = ref->getSize()->getVal();

    // For each quantum register declaration, we associate a
    // number to each possible qbit.
    // 
    // For example, 'qreg q[5];' generates 'q[0]', 'q[1]', ...
    for (int i = 0; i < size.mV; ++i) {
        std::string key = id + "[" + std::to_string(i) +"]";
        auto ref = NDIdRef::Create(NDId::Create(id), NDInt::Create(std::to_string(i)));
        mQbitToNumber.mGIdMap.push_back
            (QbitToNumber::QbitInfo { key, toShared(std::move(ref)) });
    }
}

void efd::QbitToNumberVisitor::visit(NDGateDecl::Ref ref) {
    if (mQbitToNumber.mLIdMap.find(ref) == mQbitToNumber.mLIdMap.end()) {
        mQbitToNumber.mLIdMap[ref] = efd::QbitToNumber::QbitMap();

        // Each quantum argument of each quantum gate declaration
        // will be mapped to a number.
        for (auto& childRef : *ref->getQArgs()) {
            NDId::Ref idref = dynCast<NDId>(childRef.get());
            mQbitToNumber.mLIdMap[ref].push_back
                (QbitToNumber::QbitInfo { idref->getVal(), toShared(idref->clone()) });
        }
    }
}

void efd::QbitToNumberWrapperPass::run(QModule::Ref qmod) {
    mData.mGIdMap.clear();
    mData.mLIdMap.clear();

    QbitToNumberVisitor visitor(mData);

    for (auto it = qmod->reg_begin(), e = qmod->reg_end(); it != e; ++it) {
        (*it)->apply(&visitor);
    }

    for (auto it = qmod->gates_begin(), e = qmod->gates_end(); it != e; ++it) {
        (*it)->apply(&visitor);
    }
}

efd::QbitToNumberWrapperPass::uRef efd::QbitToNumberWrapperPass::Create() {
    return uRef(new QbitToNumberWrapperPass());
}
