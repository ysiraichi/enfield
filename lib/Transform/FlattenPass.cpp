#include "enfield/Transform/FlattenPass.h"
#include "enfield/Analysis/NodeVisitor.h"
#include "enfield/Support/RTTI.h"
#include "enfield/Support/uRefCast.h"

#include <limits>

using namespace efd;

namespace efd {
    class FlattenVisitor;
}

static bool IsIdRef(Node::Ref ref);
static bool IsId(Node::Ref ref);
static bool HasAnyNDIdChild(Node::Ref ref);

static NDRegDecl::Ref GetDeclFromId(const QModule::Ref qmod, Node::Ref ref);
static uint32_t GetIdDeclSize(const QModule::Ref qmod, Node::Ref ref);
static std::vector<NDIdRef::uRef> ToIdRef(const QModule::Ref qmod, Node::Ref ref, uint32_t max);
static std::vector<std::vector<NDIdRef::uRef>>
GetFlattenedOpsArgs(const QModule::Ref qmod, std::vector<Node::Ref> qcargs);

static Node::uRef WrapWithIfStmt(Node::Ref ifnode, Node::uRef node);
static void FlattenVisitQOperation(FlattenVisitor* visitor, Node::Ref ifstmt, NDQOp::Ref ref);

uint8_t efd::FlattenPass::ID = 0;

namespace efd {
    class FlattenVisitor : public NodeVisitor {
        private:
            QModule& mMod;
            Node::Ref mIf;

        public:
            std::unordered_map<Node::Ref, std::vector<Node::uRef>> mRepMap;

            FlattenVisitor(QModule& qmod) : mMod(qmod), mIf(nullptr) {}

            QModule::Ref getQMod() const;

            void visit(NDQOpBarrier::Ref ref) override;
            void visit(NDQOpMeasure::Ref ref) override;
            void visit(NDQOpReset::Ref ref) override;
            void visit(NDQOpCX::Ref ref) override;
            void visit(NDQOpU::Ref ref) override;
            void visit(NDQOpGen::Ref ref) override;
            void visit(NDIfStmt::Ref ref) override;
    };
}

QModule::Ref efd::FlattenVisitor::getQMod() const {
    return &mMod;
}

void efd::FlattenVisitor::visit(NDQOpBarrier::Ref ref) {
    FlattenVisitQOperation(this, mIf, (NDQOp::Ref) ref);
}

void efd::FlattenVisitor::visit(NDQOpMeasure::Ref ref) {
    if (HasAnyNDIdChild(ref))
        return;

    Node::Ref key = (mIf == nullptr) ? (Node::Ref) ref : mIf;

    std::vector<Node::uRef> newNodes;
    auto flatArgs = GetFlattenedOpsArgs(&mMod, { ref->getQBit(), ref->getCBit() });

    if (!flatArgs.empty()) {

        for (uint32_t i = 0, e = flatArgs[0].size(); i < e; ++i) {
            auto qop = NDQOpMeasure::Create(std::move(flatArgs[0][i]), std::move(flatArgs[1][i]));
            newNodes.push_back(WrapWithIfStmt(mIf, std::move(qop)));
        }

        mRepMap[key] = std::move(newNodes);;
    }
}

void efd::FlattenVisitor::visit(NDQOpReset::Ref ref) {
    FlattenVisitQOperation(this, mIf, (NDQOp::Ref) ref);
}

void efd::FlattenVisitor::visit(NDQOpU::Ref ref) {
    FlattenVisitQOperation(this, mIf, (NDQOp::Ref) ref);
}

void efd::FlattenVisitor::visit(NDQOpCX::Ref ref) {
    FlattenVisitQOperation(this, mIf, (NDQOp::Ref) ref);
}

void efd::FlattenVisitor::visit(NDQOpGen::Ref ref) {
    FlattenVisitQOperation(this, mIf, (NDQOp::Ref) ref);
}

void efd::FlattenVisitor::visit(NDIfStmt::Ref ref) {
    mIf = ref;
    visitChildren(ref);
    mIf = nullptr;
}

efd::FlattenPass::FlattenPass() {
}

bool efd::FlattenPass::run(QModule::Ref qmod) {
    FlattenVisitor visitor(*qmod);

    for (auto it = qmod->stmt_begin(), e = qmod->stmt_end(); it != e; ++it) {
        (*it)->apply(&visitor);
    }

    for (auto& pair : visitor.mRepMap) {
        qmod->replaceStatement(pair.first, std::move(pair.second));
    }

    return true;
}

FlattenPass::uRef efd::FlattenPass::Create() {
    return uRef(new FlattenPass());
}

// ----------------------------------------------------------------
// --------------------- Static Functions -------------------------
// ----------------------------------------------------------------

static bool IsIdRef(Node::Ref ref) {
    return instanceOf<NDIdRef>(ref);
}

static bool IsId(Node::Ref ref) {
    return instanceOf<NDId>(ref);
}

static bool HasAnyNDIdChild(Node::Ref ref) {
    for (auto& child : *ref)
        if (IsId(child.get())) return false;
    return true;
}

static NDRegDecl::Ref GetDeclFromId(const QModule::Ref qmod, Node::Ref ref) {
    NDId::Ref refId = dynCast<NDId>(ref);
    EfdAbortIf(refId == nullptr, "Not an Id: `" << ref->toString(false) << "`.");

    auto node = qmod->getQVar(refId->getVal());
    NDRegDecl::Ref refDecl = dynCast<NDRegDecl>(node);

    EfdAbortIf(refDecl == nullptr,
               "Not an NDRegDecl: `" << node->toString(false) << "`.");

    return refDecl;
}

static uint32_t GetIdDeclSize(const QModule::Ref qmod, Node::Ref ref) {
    return GetDeclFromId(qmod, ref)->getSize()->getVal().mV;
}

static std::vector<NDIdRef::uRef> ToIdRef(const QModule::Ref qmod, Node::Ref ref, uint32_t max) {
    std::vector<NDIdRef::uRef> idRefV;

    if (IsIdRef(ref)) {
        for (uint32_t i = 0; i < max; ++i)
            idRefV.push_back(uniqueCastForward<NDIdRef>(ref->clone()));
        return idRefV;
    }

    NDRegDecl::Ref refDecl = GetDeclFromId(qmod, ref);
    uint32_t i = 0, e = refDecl->getSize()->getVal().mV;

    if (max != 0) e = std::max(max, e);
    for (; i < e; ++i) {
        std::string strVal = std::to_string(i);
        idRefV.push_back(NDIdRef::Create
                (uniqueCastForward<NDId>(ref->clone()), NDInt::Create(strVal)));
    }

    return idRefV;
}

static std::vector<std::vector<NDIdRef::uRef>>
GetFlattenedOpsArgs(const QModule::Ref qmod, std::vector<Node::Ref> qcargs) {
    std::vector<std::vector<NDIdRef::uRef>> newNodesArgs;
    const uint32_t uint32max = std::numeric_limits<uint32_t>::max();

    uint32_t min = uint32max;

    for (auto child : qcargs) {
        if (IsId(child)) {
            auto declsize = GetIdDeclSize(qmod, child);
            if (declsize < min) min = declsize;
        }
    }

    if (min != uint32max) {
        for (auto child : qcargs) {
            newNodesArgs.push_back(ToIdRef(qmod, child, min));
        }
    }

    return newNodesArgs;
}

static Node::uRef WrapWithIfStmt(Node::Ref ifnode, Node::uRef node) {
    auto wrapped = std::move(node);

    if (ifnode != nullptr) {
        auto ifstmt = uniqueCastForward<NDIfStmt>(ifnode->clone());
        ifstmt->setQOp(uniqueCastForward<NDQOp>(std::move(wrapped)));
        wrapped = std::move(ifstmt);
    }

    return wrapped;
}

static void FlattenVisitQOperation(FlattenVisitor* visitor, Node::Ref ifstmt, NDQOp::Ref ref) {
    if (HasAnyNDIdChild(ref->getQArgs()))
        return;

    std::vector<Node::uRef> newNodes;

    QModule::Ref qmod = visitor->getQMod();
    Node::Ref key = (ifstmt == nullptr) ? (Node::Ref) ref : ifstmt;

    std::vector<Node::Ref> qcargs;
    for (auto& child : *ref->getQArgs()) qcargs.push_back(child.get());

    auto flatArgs = GetFlattenedOpsArgs(qmod, qcargs);

    if (!flatArgs.empty()) {
        for (uint32_t i = 0, e = flatArgs[0].size(); i < e; ++i) {
            auto qaList = NDList::Create();

            for (auto& qarg : flatArgs)
                qaList->addChild(std::move(qarg[i]));

            auto clone = uniqueCastForward<NDQOp>(ref->clone());
            clone->setQArgs(std::move(qaList));
            newNodes.push_back(WrapWithIfStmt(ifstmt, std::move(clone)));
        }

        visitor->mRepMap[key] = std::move(newNodes);;
    }
}
