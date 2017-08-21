#include "enfield/Transform/FlattenPass.h"
#include "enfield/Analysis/NodeVisitor.h"
#include "enfield/Support/RTTI.h"
#include "enfield/Support/uRefCast.h"

#include <cassert>
#include <limits>

namespace efd {
    class FlattenVisitor : public NodeVisitor {
        private:
            QModule& mMod;
            NDIfStmt::Ref mIf;

            /// \brief Returns true if \p ref is an IdRef.
            bool isIdRef(Node::Ref ref);
            /// \brief Returns true if \p ref is an Id.
            bool isId(Node::Ref ref);

            /// \brief Gets the declaration node from an Id node.
            NDRegDecl::Ref getDeclFromId(Node::Ref ref);
            /// \brief Creates \p max NDIdRef's related to that Id. If \p max
            /// is 0, then create all of them.
            std::vector<NDIdRef::uRef> toIdRef(Node::Ref ref, unsigned max = 0);

            /// \brief Returns the size of the declaration of this Id node.
            unsigned getSize(Node::Ref ref);
            /// \brief Returns a list with all IdRef's possible of all QArgs.
            std::vector<std::vector<NDIdRef::uRef>> getFlattenedOpsArgs(Node::Ref ref);
            /// \brief Returns true if all the childrem are IdRef node.
            bool isChildremIdRef(Node::Ref ref);

            /// \brief Wraps \p node if we are currently inside an if statement.
            Node::uRef wrapWithIfStmt(Node::uRef node);

        public:
            std::unordered_map<Node::Ref, std::vector<Node::uRef>> mRepMap;

            FlattenVisitor(QModule& qmod) : mMod(qmod), mIf(nullptr) {}

            void visit(NDQOpBarrier::Ref ref) override;
            void visit(NDQOpMeasure::Ref ref) override;
            void visit(NDQOpReset::Ref ref) override;
            void visit(NDQOpCX::Ref ref) override;
            void visit(NDQOpGeneric::Ref ref) override;
            void visit(NDIfStmt::Ref ref) override;
    };
}

bool efd::FlattenVisitor::isIdRef(Node::Ref ref) {
    return instanceOf<NDIdRef>(ref);
}

bool efd::FlattenVisitor::isId(Node::Ref ref) {
    return instanceOf<NDId>(ref);
}

efd::NDRegDecl::Ref efd::FlattenVisitor::getDeclFromId(Node::Ref ref) {
    NDId::Ref refId = dynCast<NDId>(ref);
    assert(refId != nullptr && "Malformed statement.");

    NDRegDecl::Ref refDecl = dynCast<NDRegDecl>(mMod.getQVar(refId->getVal()));
    assert(refDecl != nullptr && "No such qubit declared.");

    return refDecl;
}

std::vector<efd::NDIdRef::uRef> efd::FlattenVisitor::toIdRef(Node::Ref ref, unsigned max) {
    std::vector<NDIdRef::uRef> idRefV;

    if (isIdRef(ref)) {
        for (unsigned i = 0; i < max; ++i)
            idRefV.push_back(uniqueCastForward<NDIdRef>(ref->clone()));
        return idRefV;
    }

    NDRegDecl::Ref refDecl = getDeclFromId(ref);
    unsigned i = 0, e = refDecl->getSize()->getVal().mV;

    if (max != 0) e = std::max(max, e);
    for (; i < e; ++i) {
        std::string strVal = std::to_string(i);
        idRefV.push_back(NDIdRef::Create
                (uniqueCastForward<NDId>(ref->clone()), NDInt::Create(strVal)));
    }

    return idRefV;
}

unsigned efd::FlattenVisitor::getSize(Node::Ref ref) {
    return getDeclFromId(ref)->getSize()->getVal().mV;
}

std::vector<std::vector<efd::NDIdRef::uRef>> efd::FlattenVisitor::getFlattenedOpsArgs(Node::Ref ref) {
    std::vector<std::vector<NDIdRef::uRef>> newNodesArgs;

    unsigned min = std::numeric_limits<unsigned>::max();
    for (auto& childRef : *ref) {
        auto child = childRef.get();
        if (isId(child) && getSize(child) < min)
            min = getSize(child);
    }

    for (auto& child : *ref) {
        newNodesArgs.push_back(toIdRef(child.get(), min));
    }

    return newNodesArgs;
}

bool efd::FlattenVisitor::isChildremIdRef(Node::Ref ref) {
    for (auto& child : *ref)
        if (isId(child.get()))
            return false;
    return true;
}

efd::Node::uRef efd::FlattenVisitor::wrapWithIfStmt(Node::uRef node) {
    auto wrapped = std::move(node);

    if (mIf != nullptr) {
        auto ifstmt = uniqueCastForward<NDIfStmt>(mIf->clone());
        ifstmt->setQOp(std::move(wrapped));
        wrapped = std::move(ifstmt);
    }

    return wrapped;
}

void efd::FlattenVisitor::visit(NDQOpBarrier::Ref ref) {
    if (isChildremIdRef(ref->getQArgs()))
        return;

    Node::Ref key = (mIf == nullptr) ? (Node::Ref) ref : (Node::Ref) mIf;

    std::vector<Node::uRef> newNodes;
    auto flatArgs = getFlattenedOpsArgs(ref->getQArgs());
    for (unsigned i = 0, e = flatArgs[0].size(); i < e; ++i) {
        auto qargs = NDList::Create();

        for (unsigned j = 0, f = flatArgs.size(); j < f; ++j)
            qargs->addChild(std::move(flatArgs[j][i]));

        newNodes.push_back(wrapWithIfStmt(NDQOpBarrier::Create(std::move(qargs))));
    }

    mRepMap[key] = std::move(newNodes);;
}

void efd::FlattenVisitor::visit(NDQOpMeasure::Ref ref) {
    if (isChildremIdRef(ref))
        return;

    Node::Ref key = (mIf == nullptr) ? (Node::Ref) ref : (Node::Ref) mIf;

    std::vector<Node::uRef> newNodes;
    auto flatArgs = getFlattenedOpsArgs(ref);
    for (unsigned i = 0, e = flatArgs[0].size(); i < e; ++i)
        newNodes.push_back(wrapWithIfStmt
                (NDQOpMeasure::Create(std::move(flatArgs[0][i]),
                                      std::move(flatArgs[1][i]))));

    mRepMap[key] = std::move(newNodes);;
}

void efd::FlattenVisitor::visit(NDQOpReset::Ref ref) {
    if (isChildremIdRef(ref))
        return;

    Node::Ref key = (mIf == nullptr) ? (Node::Ref) ref : (Node::Ref) mIf;

    std::vector<Node::uRef> newNodes;
    auto flatArgs = getFlattenedOpsArgs(ref);
    for (unsigned i = 0, e = flatArgs[0].size(); i < e; ++i)
        newNodes.push_back(wrapWithIfStmt(NDQOpReset::Create(std::move(flatArgs[0][i]))));

    mRepMap[key] = std::move(newNodes);;
}

void efd::FlattenVisitor::visit(NDQOpCX::Ref ref) {
    if (isChildremIdRef(ref))
        return;

    Node::Ref key = (mIf == nullptr) ? (Node::Ref) ref : (Node::Ref) mIf;

    std::vector<Node::uRef> newNodes;
    auto flatArgs = getFlattenedOpsArgs(ref);
    for (unsigned i = 0, e = flatArgs[0].size(); i < e; ++i)
        newNodes.push_back(wrapWithIfStmt
               (NDQOpCX::Create(std::move(flatArgs[0][i]), std::move(flatArgs[1][i]))));

    mRepMap[key] = std::move(newNodes);;
}

void efd::FlattenVisitor::visit(NDQOpGeneric::Ref ref) {
    if (isChildremIdRef(ref->getQArgs()))
        return;

    Node::Ref key = (mIf == nullptr) ? (Node::Ref) ref : (Node::Ref) mIf;

    std::vector<Node::uRef> newNodes;
    auto flatArgs = getFlattenedOpsArgs(ref->getQArgs());
    for (unsigned i = 0, e = flatArgs[0].size(); i < e; ++i) {
        auto qaList = NDList::Create();

        for (auto& qarg : flatArgs)
            qaList->addChild(std::move(qarg[i]));

        newNodes.push_back(wrapWithIfStmt
                (NDQOpGeneric::Create(
                    uniqueCastForward<NDId>(ref->getId()->clone()),
                    uniqueCastForward<NDList>(ref->getArgs()->clone()),
                    std::move(qaList))));
    }

    mRepMap[key] = std::move(newNodes);;
}

void efd::FlattenVisitor::visit(NDIfStmt::Ref ref) {
    mIf = ref;
    visitChildren(ref);
    mIf = nullptr;
}

efd::FlattenPass::FlattenPass() {
}

void efd::FlattenPass::run(QModule::Ref qmod) {
    FlattenVisitor visitor(*qmod);

    for (auto it = qmod->stmt_begin(), e = qmod->stmt_end(); it != e; ++it) {
        (*it)->apply(&visitor);
    }

    for (auto& pair : visitor.mRepMap) {
        qmod->replaceStatement(pair.first, std::move(pair.second));
    }
}

efd::FlattenPass::uRef efd::FlattenPass::Create() {
    return uRef(new FlattenPass());
}

