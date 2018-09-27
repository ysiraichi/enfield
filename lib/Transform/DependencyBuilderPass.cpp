#include "enfield/Transform/DependencyBuilderPass.h"
#include "enfield/Transform/PassCache.h"
#include "enfield/Analysis/NodeVisitor.h"
#include "enfield/Support/RTTI.h"
#include "enfield/Support/Defs.h"

#include <algorithm>

// --------------------- Dependencies ------------------------
bool efd::Dependencies::empty() const {
    return mDeps.empty();
}

uint32_t efd::Dependencies::size() const {
    return mDeps.size();
}

const efd::Dep& efd::Dependencies::operator[](uint32_t i) const {
    return mDeps[i];
}

efd::Dep& efd::Dependencies::operator[](uint32_t i) {
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

// --------------------- DependencyBuilder ------------------------
efd::DependencyBuilder::DependencyBuilder() {
}

uint32_t efd::DependencyBuilder::getUId(Node::Ref ref, NDGateDecl::Ref gate) {
    std::string _id = ref->toString();
    return mXbitToNumber.getQUId(_id, gate);
}

const efd::DependencyBuilder::DepsVector* efd::DependencyBuilder::getDepsVector
(NDGateDecl::Ref gate) const {
    const DepsVector* deps = &mGDeps;

    if (gate != nullptr) {
        EfdAbortIf(mLDeps.find(gate) == mLDeps.end(),
                   "No dependencies for this gate: `" << gate->getId()->getVal() << "`.");

        deps = &mLDeps.at(gate);
    }

    return deps;
}

efd::DependencyBuilder::DepsVector* efd::DependencyBuilder::getDepsVector
(NDGateDecl::Ref gate) {
    return const_cast<DepsVector*>(static_cast<const DependencyBuilder*>
            (this)->getDepsVector(gate));
}

efd::XbitToNumber& efd::DependencyBuilder::getXbitToNumber() {
    return mXbitToNumber;
}

void efd::DependencyBuilder::setXbitToNumber(efd::XbitToNumber& xtn) {
    mXbitToNumber = xtn;
}

const efd::DependencyBuilder::DepsVector& efd::DependencyBuilder::getDependencies
(NDGateDecl::Ref ref) const {
    const DepsVector* deps = const_cast<DepsVector*>(getDepsVector(ref));
    return *deps;
}

efd::DependencyBuilder::DepsVector& efd::DependencyBuilder::getDependencies
(NDGateDecl::Ref ref) {
    return *getDepsVector(ref);
}

const efd::Dependencies efd::DependencyBuilder::getDeps(Node* ref) const {
    EfdAbortIf(mIDeps.find(ref) == mIDeps.end(),
               "Instruction never seen before: `" <<
               ((ref == nullptr) ? "nullptr" : ref->toString(false))
               << "`.");

    return mIDeps.at(ref);
}

efd::Dependencies efd::DependencyBuilder::getDeps(Node* ref) {
    if (mIDeps.find(ref) == mIDeps.end())
        mIDeps[ref] = { {}, ref };
    return mIDeps.at(ref);
}

// --------------------- DependencyBuilderWrapperPass ------------------------
uint8_t efd::DependencyBuilderWrapperPass::ID = 0;

namespace efd {
    class DependencyBuilderVisitor : public NodeVisitor {
        private:
            QModule& mMod;
            DependencyBuilder& mDepBuilder;

            /// \brief Gets a reference to the parent gate it is in. If it is not
            /// in any gate, it returns a nullptr.
            NDGateDecl::Ref getParentGate(Node::Ref ref);

        public:
            DependencyBuilderVisitor(QModule& qmod, DependencyBuilder& dep)
                : mMod(qmod), mDepBuilder(dep) {}

            void visit(NDGateDecl::Ref ref) override;
            void visit(NDQOpCX::Ref ref) override;
            void visit(NDQOpGen::Ref ref) override;
            void visit(NDIfStmt::Ref ref) override;
            void visit(NDGOpList::Ref ref) override;
    };
}

efd::NDGateDecl::Ref efd::DependencyBuilderVisitor::getParentGate(Node::Ref ref) {
    NDGOpList::Ref goplist = nullptr;

    if (auto ifstmt = dynCast<NDIfStmt>(ref->getParent())) {
        goplist = dynCast<NDGOpList>(ifstmt->getParent());
    } else {
        goplist = dynCast<NDGOpList>(ref->getParent());
    }

    if (goplist == nullptr) {
        return nullptr;
    }

    auto gate = dynCast<NDGateDecl>(goplist->getParent());
    EfdAbortIf(gate == nullptr, "NDGOpList is owned by a no node.");
    return gate;
}

void efd::DependencyBuilderVisitor::visit(NDGateDecl::Ref ref) {
    mDepBuilder.mLDeps[ref] = DependencyBuilder::DepsVector();
    visitChildren(ref);
}

void efd::DependencyBuilderVisitor::visit(NDQOpCX::Ref ref) {
    auto gate = getParentGate(ref);
    auto deps = mDepBuilder.getDepsVector(gate);

    // CX controlQ, invertQ;
    uint32_t controlQ = mDepBuilder.getUId(ref->getLhs(), gate);
    uint32_t invertQ = mDepBuilder.getUId(ref->getRhs(), gate);

    Dependencies depV { { Dep { controlQ, invertQ } }, ref };

    deps->push_back(depV);
    mDepBuilder.mIDeps[ref] = depV;
}

void efd::DependencyBuilderVisitor::visit(NDQOpGen::Ref ref) {
    // Single qbit gate.
    if (ref->getQArgs()->getChildNumber() == 1) return;

    auto gate = getParentGate(ref);
    auto deps = mDepBuilder.getDepsVector(gate);

    // Getting the qargs uint32_t representations.
    std::vector<uint32_t> uidVector;
    for (auto& childRef : *ref->getQArgs())
        uidVector.push_back(mDepBuilder.getUId(childRef.get(), gate));

    // Getting the gate declaration node.
    Node::Ref node = mMod.getQGate(ref->getId()->getVal());
    NDGateDecl::Ref gRef = dynCast<NDGateDecl>(node);

    EfdAbortIf(gRef == nullptr,
               "There is no quantum gate with this id: `" << node->toString(false) << "`.");

    auto& gDeps = mDepBuilder.mLDeps[gRef];
    Dependencies thisDeps { {}, ref };
    // For every qarg uint32_t representation
    for (auto parallelDeps : gDeps) {
        for (auto dep : parallelDeps) {
            // Getting the uid's of the qubit interaction (u, v)
            uint32_t u = uidVector[dep.mFrom];
            uint32_t v = uidVector[dep.mTo];
            thisDeps.mDeps.push_back(Dep { u, v });
        }
    }

    if (!thisDeps.empty())
        deps->push_back(thisDeps);
    mDepBuilder.mIDeps[ref] = thisDeps;
}

void efd::DependencyBuilderVisitor::visit(NDIfStmt::Ref ref) {
    mDepBuilder.mIDeps[ref] = Dependencies();
    visitChildren(ref);
}

void efd::DependencyBuilderVisitor::visit(NDGOpList::Ref ref) {
    visitChildren(ref);
}

bool efd::DependencyBuilderWrapperPass::run(QModule::Ref qmod) {
    mData.mLDeps.clear();
    mData.mGDeps.clear();
    mData.mIDeps.clear();

    auto xtn = PassCache::Get<XbitToNumberWrapperPass>(qmod);
    auto data = xtn->getData();
    mData.setXbitToNumber(data);

    DependencyBuilderVisitor visitor(*qmod, mData);
    for (auto it = qmod->gates_begin(), e = qmod->gates_end(); it != e; ++it) {
        (*it)->apply(&visitor);
    }

    for (auto it = qmod->stmt_begin(), e = qmod->stmt_end(); it != e; ++it) {
        (*it)->apply(&visitor);
    }

    return false;
}

efd::DependencyBuilderWrapperPass::uRef efd::DependencyBuilderWrapperPass::Create() {
    return uRef(new DependencyBuilderWrapperPass());
}
