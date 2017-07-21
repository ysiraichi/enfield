#include "enfield/Transform/FlattenPass.h"
#include "enfield/Support/RTTI.h"
#include "enfield/Support/uRefCast.h"

#include <cassert>
#include <limits>

efd::FlattenPass::FlattenPass(QModule::sRef qmod) : mMod(qmod) {
    mUK += Pass::K_STMT_PASS;
}

bool efd::FlattenPass::isIdRef(Node::Ref ref) {
    return instanceOf<NDIdRef>(ref);
}

bool efd::FlattenPass::isId(Node::Ref ref) {
    return instanceOf<NDId>(ref);
}

efd::NDRegDecl::Ref efd::FlattenPass::getDeclFromId(Node::Ref ref) {
    NDId::Ref refId = dynCast<NDId>(ref);
    assert(refId != nullptr && "Malformed statement.");

    NDRegDecl::Ref refDecl = dynCast<NDRegDecl>(mMod->getQVar(refId->getVal()));
    assert(refDecl != nullptr && "No such qubit declared.");

    return refDecl;
}

std::vector<efd::NDIdRef::uRef> efd::FlattenPass::toIdRef(Node::Ref ref, unsigned max) {
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

void efd::FlattenPass::replace(Node::Ref ref, std::vector<Node::uRef> nodes) {
    NDList::Ref list = dynCast<NDList>(ref->getParent());

    if (list == nullptr) {
        auto parent = dynCast<NDIfStmt>(ref->getParent());
        assert(parent != nullptr && "Parent should be either IfStmt or List.");

        list = dynCast<NDList>(parent->getParent());
        assert(list != nullptr && "We should end up in a List.");

        for (unsigned i = 0, e = nodes.size(); i < e; ++i) {
            auto ifstmt = uniqueCastForward<NDIfStmt>(parent->clone());
            ifstmt->setQOp(std::move(nodes[i]));
            nodes[i] = uniqueCastBackward<Node>(std::move(ifstmt));
        }
    }

    auto It = list->findChild(ref);
    for (auto& child : nodes) {
        list->addChild(It, child->clone());
        ++It;
    }
    list->removeChild(ref);
}

unsigned efd::FlattenPass::getSize(Node::Ref ref) {
    return getDeclFromId(ref)->getSize()->getVal().mV;
}

std::vector<std::vector<efd::NDIdRef::uRef>> efd::FlattenPass::getFlattenedOpsArgs(Node::Ref ref) {
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

bool efd::FlattenPass::isChildremIdRef(Node::Ref ref) {
    for (auto& child : *ref)
        if (isId(child.get()))
            return false;
    return true;
}

void efd::FlattenPass::visit(NDQOpBarrier::Ref ref) {
    if (isChildremIdRef(ref->getQArgs()))
        return;

    std::vector<Node::uRef> newNodes;
    auto flatArgs = getFlattenedOpsArgs(ref->getQArgs());
    for (unsigned i = 0, e = flatArgs[0].size(); i < e; ++i) {
        auto qargs = NDList::Create();

        for (unsigned j = 0, f = flatArgs.size(); j < f; ++j)
            qargs->addChild(std::move(flatArgs[j][i]));
        newNodes.push_back(uniqueCastBackward<Node>
                (NDQOpBarrier::Create(std::move(qargs))));
    }
    replace(ref, std::move(newNodes));
}

void efd::FlattenPass::visit(NDQOpMeasure::Ref ref) {
    if (isChildremIdRef(ref))
        return;

    std::vector<Node::uRef> newNodes;
    auto flatArgs = getFlattenedOpsArgs(ref);
    for (unsigned i = 0, e = flatArgs[0].size(); i < e; ++i)
        newNodes.push_back(uniqueCastBackward<Node>
                (NDQOpMeasure::Create
                 (std::move(flatArgs[0][i]), std::move(flatArgs[1][i]))));
    replace(ref, std::move(newNodes));
}

void efd::FlattenPass::visit(NDQOpReset::Ref ref) {
    if (isChildremIdRef(ref))
        return;

    std::vector<Node::uRef> newNodes;
    auto flatArgs = getFlattenedOpsArgs(ref);
    for (unsigned i = 0, e = flatArgs[0].size(); i < e; ++i)
        newNodes.push_back(uniqueCastBackward<Node>
                (NDQOpReset::Create(std::move(flatArgs[0][i]))));
    replace(ref, std::move(newNodes));
}

void efd::FlattenPass::visit(NDQOpCX::Ref ref) {
    if (isChildremIdRef(ref))
        return;

    std::vector<Node::uRef> newNodes;
    auto flatArgs = getFlattenedOpsArgs(ref);
    for (unsigned i = 0, e = flatArgs[0].size(); i < e; ++i)
        newNodes.push_back(uniqueCastBackward<Node>
               (NDQOpCX::Create(std::move(flatArgs[0][i]), std::move(flatArgs[1][i]))));
    replace(ref, std::move(newNodes));
}

void efd::FlattenPass::visit(NDQOpGeneric::Ref ref) {
    if (isChildremIdRef(ref->getQArgs()))
        return;

    std::vector<Node::uRef> newNodes;
    auto flatArgs = getFlattenedOpsArgs(ref->getQArgs());
    for (unsigned i = 0, e = flatArgs[0].size(); i < e; ++i) {
        auto qaList = NDList::Create();

        for (auto& qarg : flatArgs)
            qaList->addChild(std::move(qarg[i]));

        newNodes.push_back(NDQOpGeneric::Create(
                    uniqueCastForward<NDId>(ref->getId()->clone()),
                    uniqueCastForward<NDList>(ref->getArgs()->clone()),
                    std::move(qaList)));
    }
    replace(ref, std::move(newNodes));
}

void efd::FlattenPass::initImpl(bool force) {
    mIfNewNodes.clear();
}

bool efd::FlattenPass::doesInvalidatesModule() const {
    return true;
}

efd::FlattenPass::uRef efd::FlattenPass::Create(QModule::sRef qmod) {
    return uRef(new FlattenPass(qmod));
}

