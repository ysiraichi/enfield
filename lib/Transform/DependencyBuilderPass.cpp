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

efd::QbitToNumberPass* efd::QbitToNumberPass::Create() {
    return new QbitToNumberPass();
}

// --------------------- Dependencies ------------------------
bool efd::Dependencies::isEmpty() const {
    return mDeps.empty();
}

unsigned efd::Dependencies::getSize() const {
    return mDeps.size();
}

const efd::Dep& efd::Dependencies::operator[](unsigned i) const {
    return mDeps[i];
}

efd::Dep& efd::Dependencies::operator[](unsigned i) {
    return mDeps[i];
}

efd::Dependencies::Iterator efd::Dependencies::begin() {
    return mDeps.begin();
}

efd::Dependencies::ConstIterator efd::Dependencies::begin() const {
    return mDeps.begin();
}

efd::Dependencies::Iterator efd::Dependencies::end() {
    return mDeps.end();
}

efd::Dependencies::ConstIterator efd::Dependencies::end() const {
    return mDeps.end();
}

// --------------------- DependencyBuilderPass ------------------------
efd::DependencyBuilderPass::DependencyBuilderPass(QModule* mod, QbitToNumberPass* pass) 
    : mMod(mod), mQbitMap(pass) {
    if (mQbitMap == nullptr)
        mQbitMap = QbitToNumberPass::Create();

    mUK += Pass::K_GATE_PASS;
    mUK += Pass::K_STMT_PASS;
}

void efd::DependencyBuilderPass::initImpl() {
    mCurrentGate = nullptr;
    mLDeps.clear();
    mGDeps.clear();

    mMod->runPass(mQbitMap);
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
    mLDeps[ref] = DepsSet();

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

    Dependencies depV { { Dep { controlQ, invertQ } }, nullptr };
    deps->push_back(depV);
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
    Dependencies thisDeps;
    // For every qarg unsigned representation
    for (auto parallelDeps : gDeps) {
        for (auto dep : parallelDeps) {
            // Getting the uid's of the qubit interaction (u, v)
            unsigned u = uidVector[dep.mFrom];
            unsigned v = uidVector[dep.mTo];
            thisDeps.mDeps.push_back(Dep { u, v });
        }
    }

    if (!thisDeps.isEmpty())
        deps->push_back(thisDeps);
}

void efd::DependencyBuilderPass::visit(NDIfStmt* ref) {
    ref->getQOp()->apply(this);
}

efd::QbitToNumberPass* efd::DependencyBuilderPass::getUIdPass() {
    return mQbitMap;
}

const efd::DependencyBuilderPass::DepsSet& efd::DependencyBuilderPass::getDependencies(NDGateDecl* ref) const {
    const DepsSet* deps = const_cast<DepsSet*>(getDepsSet(ref));
    return *deps;
}

efd::DependencyBuilderPass::DepsSet efd::DependencyBuilderPass::getDependencies(NDGateDecl* ref) {
    return *getDepsSet(ref);
}

efd::DependencyBuilderPass* efd::DependencyBuilderPass::Create(QModule* mod, QbitToNumberPass* pass) {
    return new DependencyBuilderPass(mod, pass);
}
