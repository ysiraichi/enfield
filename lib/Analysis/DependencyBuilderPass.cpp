#include "enfield/Analysis/DependencyBuilderPass.h"
#include "enfield/Support/RTTI.h"

#include <cassert>
#include <algorithm>

// --------------------- QbitToNumberPass ------------------------
void efd::QbitToNumberPass::visit(NDDecl* ref) {
    if (ref->isCReg()) return;

    std::string id = ref->getId()->getVal();
    IntVal size = ref->getSize()->getVal();

    // For each quantum register declaration, we associate a
    // number to each possible qbit.
    // 
    // For example, 'qreg q[5];' generates 'q[0]', 'q[1]', ...
    for (int i = 0; i < size.mV; ++i)
        mGIdMap.push_back(id + "[" + std::to_string(i) +"]");
}

void efd::QbitToNumberPass::visit(NDGateDecl* ref) {
    if (mLIdMap.find(ref) == mLIdMap.end()) {
        mLIdMap[ref] = QbitMap();

        // Each quantum argument of each quantum gate declaration
        // will be mapped to a number.
        for (auto childRef : *ref->getQArgs()) {
            NDId* idref = dynCast<NDId>(childRef);
            mLIdMap[ref].push_back(idref->getVal());
        }
    }
}

unsigned efd::QbitToNumberPass::getUId(std::string id, NDGateDecl* gateRef) const {
    const QbitMap* map = &mGIdMap;

    if (gateRef != nullptr) {
        assert(mLIdMap.find(gateRef) != mLIdMap.end() && \
                "Gate not parsed.");
        map = &mLIdMap.at(gateRef);
    }

    unsigned index = 0;
    auto it = map->begin();
    for (auto end = map->end(); it != end; ++it, ++index)
        if (*it == id) break;

    assert(it != map->end() && "Id not found.");
    return index;
}

std::string efd::QbitToNumberPass::getStrId(unsigned id, NDGateDecl* gateRef) const {
    const QbitMap* map = &mGIdMap;

    if (gateRef != nullptr) {
        assert(mLIdMap.find(gateRef) != mLIdMap.end() && \
                "Gate not parsed.");
        map = &mLIdMap.at(gateRef);
    }

    assert(id < map->size() && "Id trying to access out of bounds value.");
    return map->at(id);
}

efd::QbitToNumberPass* efd::QbitToNumberPass::create() {
    return new QbitToNumberPass();
}

// --------------------- DependencyBuilderPass ------------------------

efd::DependencyBuilderPass::DependencyBuilderPass(QModule* mod, QbitToNumberPass* pass) 
    : mMod(mod), mQbitMap(pass) {
    if (mQbitMap == nullptr)
        mQbitMap = QbitToNumberPass::create();
}

unsigned efd::DependencyBuilderPass::getUId(NodeRef ref) {
    std::string _id;

    switch (ref->getKind()) {
        case Node::K_ID_REF:
            _id = dynCast<NDIdRef>(ref)->getId()->getVal();
            break;

        case Node::K_LIT_STRING:
            _id = dynCast<NDId>(ref)->getVal();
            break;

        default:
            assert(false && "Cannot get UID of other types.");
    }

    return mQbitMap->getUId(_id, mCurrentGate);
}

std::vector<std::set<unsigned>>* efd::DependencyBuilderPass::getCurrentDepsSet() {
    DepsSet* deps = &mGDeps;

    if (mCurrentGate != nullptr) {
        if (mLDeps.find(mCurrentGate) == mLDeps.end())
            mLDeps[mCurrentGate] = DepsSet();
        deps = &mLDeps[mCurrentGate];
    }

    return deps;
}

void efd::DependencyBuilderPass::visit(NDGateDecl* ref) {
    mCurrentGate = ref;
    ref->getGOpList()->apply(this);
    mCurrentGate = nullptr;
}

void efd::DependencyBuilderPass::visit(NDGOpList* ref) {
    for (auto childRef : *ref)
        childRef->apply(this);
}

void efd::DependencyBuilderPass::visit(NDQOpCX* ref) {
    DepsSet* deps = getCurrentDepsSet();

    // CX controlQ, invertQ;
    unsigned controlQ = getUId(ref->getLhs());
    unsigned invertQ = getUId(ref->getRhs());

    (*deps)[controlQ].insert(invertQ);
}

void efd::DependencyBuilderPass::visit(NDQOpGeneric* ref) {
    // Single qbit gate.
    if (ref->getChildNumber() == 1) return;

    DepsSet* deps = getCurrentDepsSet();

    // Getting the qargs unsigned representations.
    std::vector<unsigned> uidVector;
    for (auto childRef : *ref->getQArgs())
        uidVector.push_back(getUId(childRef));

    // Getting the gate declaration node.
    NDGateDecl* gRef = dynCast<NDGateDecl>(mMod->getQGate(ref->getId()->getVal()));
    assert(gRef != nullptr && "There is no quantum gate with this id.");

    DepsSet& gDeps = mLDeps[gRef];
    // For every qarg unsigned representation
    for (unsigned i = 0, end = gDeps.size(); i < end; ++i) {
        // The uid of the i-th qarg
        unsigned u = uidVector[i];

        // For every dependency unsigned representation
        for (auto it = gDeps[i].begin(), end = gDeps[i].end(); it != end; ++it) {
            // The uid of the it-th dependency
            unsigned v = uidVector[*it];

            // Inserting dependency (u, v)
            (*deps)[u].insert(v);
        }
    }
}

void efd::DependencyBuilderPass::init() {
    mCurrentGate = nullptr;
}

efd::DependencyBuilderPass* efd::DependencyBuilderPass::create(QModule* mod, QbitToNumberPass* pass) {
    if (pass == nullptr)
        pass = QbitToNumberPass::create();
    return new DependencyBuilderPass(mod, pass);
}
