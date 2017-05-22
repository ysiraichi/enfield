#include "enfield/Transform/DependencyBuilderPass.h"
#include "enfield/Support/RTTI.h"

#include <cassert>
#include <algorithm>

// --------------------- QbitToNumberPass ------------------------
efd::QbitToNumberPass::QbitToNumberPass() {
    mUK += Pass::K_GATE_PASS;
    mUK += Pass::K_REG_DECL_PASS;
}

const efd::QbitToNumberPass::QbitMap* efd::QbitToNumberPass::getMap(NDGateDecl* gate) const {
    const QbitMap* map = &mGIdMap;

    if (gate != nullptr) {
        assert(mLIdMap.find(gate) != mLIdMap.end() && \
                "Gate not parsed.");
        map = &mLIdMap.at(gate);
    }

    return map;
}

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

unsigned efd::QbitToNumberPass::getUId(std::string id, NDGateDecl* gate) const {
    const QbitMap* map = getMap(gate);

    unsigned index = 0;
    auto it = map->begin();
    for (auto end = map->end(); it != end; ++it, ++index)
        if (*it == id) break;

    assert(it != map->end() && "Id not found.");
    return index;
}

unsigned efd::QbitToNumberPass::getSize(NDGateDecl* gate) const {
    const QbitMap* map = getMap(gate);
    return map->size();
}

std::string efd::QbitToNumberPass::getStrId(unsigned id, NDGateDecl* gate) const {
    const QbitMap* map = getMap(gate);
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

    mUK += Pass::K_GATE_PASS;
    mUK += Pass::K_STMT_PASS;
}

void efd::DependencyBuilderPass::initImpl() {
    mCurrentGate = nullptr;
    mMod->runPass(mQbitMap);
    mGDeps.assign(mQbitMap->getSize(), std::set<unsigned>());
}

unsigned efd::DependencyBuilderPass::getUId(NodeRef ref) {
    std::string _id = ref->toString();
    return mQbitMap->getUId(_id, mCurrentGate);
}

const efd::DependencyBuilderPass::DepsSet* efd::DependencyBuilderPass::getDepsSet(NDGateDecl* gate) const {
    const DepsSet* deps = &mGDeps;

    if (gate != nullptr) {
        assert(mLDeps.find(gate) != mLDeps.end() && \
                "No dependencies for this gate.");
        deps = &mLDeps.at(gate);
    }

    return deps;
}

efd::DependencyBuilderPass::DepsSet* efd::DependencyBuilderPass::getDepsSet(NDGateDecl* gate) {
    return const_cast<DepsSet*>(static_cast<const DependencyBuilderPass*>(this)->getDepsSet(gate));
}

void efd::DependencyBuilderPass::visit(NDGateDecl* ref) {
    mLDeps[ref] = DepsSet(mQbitMap->getSize(ref), std::set<unsigned>());

    mCurrentGate = ref;
    ref->getGOpList()->apply(this);
    mCurrentGate = nullptr;
}

void efd::DependencyBuilderPass::visit(NDGOpList* ref) {
    for (auto childRef : *ref)
        childRef->apply(this);
}

void efd::DependencyBuilderPass::visit(NDQOpCX* ref) {
    DepsSet* deps = getDepsSet(mCurrentGate);

    // CX controlQ, invertQ;
    unsigned controlQ = getUId(ref->getLhs());
    unsigned invertQ = getUId(ref->getRhs());

    (*deps)[controlQ].insert(invertQ);
}

void efd::DependencyBuilderPass::visit(NDQOpGeneric* ref) {
    // Single qbit gate.
    if (ref->getChildNumber() == 1) return;

    DepsSet* deps = getDepsSet(mCurrentGate);

    // Getting the qargs unsigned representations.
    std::vector<unsigned> uidVector;
    for (auto childRef : *ref->getQArgs())
        uidVector.push_back(getUId(childRef));

    // Getting the gate declaration node.
    NodeRef node = mMod->getQGate(ref->getId()->getVal());
    NDGateDecl* gRef = dynCast<NDGateDecl>(node);
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

void efd::DependencyBuilderPass::visit(NDIfStmt* ref) {
    ref->getQOp()->apply(this);
}

const efd::DependencyBuilderPass::DepsSet& efd::DependencyBuilderPass::getDependencies(NDGateDecl* ref) const {
    const DepsSet* deps = const_cast<DepsSet*>(getDepsSet(ref));
    return *deps;
}

efd::DependencyBuilderPass* efd::DependencyBuilderPass::create(QModule* mod, QbitToNumberPass* pass) {
    if (pass == nullptr)
        pass = QbitToNumberPass::create();
    return new DependencyBuilderPass(mod, pass);
}
