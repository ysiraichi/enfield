#include "enfield/Analysis/FlattenPass.h"
#include "enfield/Support/RTTI.h"

#include <cassert>
#include <limits>

efd::FlattenPass::FlattenPass(QModule* qmod) : mMod(qmod) {
    mUK += Pass::K_STMT_PASS;
}

bool efd::FlattenPass::isIdRef(NodeRef ref) {
    return instanceOf<NDIdRef>(ref);
}

bool efd::FlattenPass::isId(NodeRef ref) {
    return instanceOf<NDId>(ref);
}

efd::NDDecl* efd::FlattenPass::getDeclFromId(NodeRef ref) {
    NDId* refId = dynCast<NDId>(ref);
    assert(refId != nullptr && "Malformed statement.");

    NDDecl* refDecl = dynCast<NDDecl>(mMod->getQVar(refId->getVal()));
    assert(refDecl != nullptr && "No such qubit declared.");

    return refDecl;
}

std::vector<efd::NodeRef> efd::FlattenPass::toIdRef(NodeRef ref, unsigned max) {
    std::vector<NodeRef> idRefV;

    if (isIdRef(ref)) {
        idRefV.push_back(ref);

        if (max != 0) {
            for (unsigned i = 1; i < max; ++i)
                idRefV.push_back(ref->clone());
        }

        return idRefV;
    }

    NDDecl* refDecl = getDeclFromId(ref);
    unsigned i = 0, e = refDecl->getSize()->getVal().mV;

    if (max != 0) e = std::max(max, e);
    for (; i < e; ++i) {
        std::string strVal = std::to_string(i);
        idRefV.push_back(NDIdRef::Create(ref, NDInt::Create(strVal)));
    }

    return idRefV;
}

void efd::FlattenPass::replace(NodeRef ref, std::vector<NodeRef> nodes) {
    if (NDList* parent = dynCast<NDList>(ref->getParent())) {
        auto It = parent->findChild(ref);
        for (auto child : nodes) {
            parent->addChild(It, child);
            ++It;
        }
        parent->removeChild(ref);
    } else if (NDIfStmt* parent = dynCast<NDIfStmt>(ref->getParent())) {
        mIfNewNodes = nodes;
    } else {
        assert(false && "Unreacheable. Node must be either If or List.");
    }
}

unsigned efd::FlattenPass::getSize(NodeRef ref) {
    return getDeclFromId(ref)->getSize()->getVal().mV;
}

std::vector<std::vector<efd::NodeRef>> efd::FlattenPass::getFlattenedOpsArgs(NodeRef ref) {
    std::vector<std::vector<NodeRef>> newNodesArgs;

    unsigned min = std::numeric_limits<unsigned>::max();
    for (auto child : *ref) {
        if (isId(child) && getSize(child) < min)
            min = getSize(child);
    }

    for (auto child : *ref) {
        newNodesArgs.push_back(toIdRef(child, min));
    }

    return newNodesArgs;
}

bool efd::FlattenPass::isChildremIdRef(NodeRef ref) {
    for (auto child : *ref)
        if (isId(child))
            return false;
    return true;
}

void efd::FlattenPass::visit(NDQOpCX* ref) {
    if (isChildremIdRef(ref))
        return;

    std::vector<NodeRef> newNodes;
    auto flatArgs = getFlattenedOpsArgs(ref);
    for (unsigned i = 0, e = flatArgs[0].size(); i < e; ++i)
        newNodes.push_back(NDQOpCX::Create(flatArgs[0][i], flatArgs[1][i]));
    replace(ref, newNodes);
}

void efd::FlattenPass::visit(NDQOpGeneric* ref) {
    if (isChildremIdRef(ref->getQArgs()))
        return;

    std::vector<NodeRef> newNodes;
    auto flatArgs = getFlattenedOpsArgs(ref->getQArgs());
    for (unsigned i = 0, e = flatArgs[0].size(); i < e; ++i) {
        NDList* qaList = dynCast<NDList>(NDList::Create());

        for (auto qarg : flatArgs)
            qaList->addChild(qarg[i]);

        newNodes.push_back(NDQOpGeneric::Create
                (ref->getId()->clone(), ref->getArgs()->clone(), qaList));
    }
    replace(ref, newNodes);
}

void efd::FlattenPass::visit(NDIfStmt* ref) {
    ref->getQOp()->apply(this);

    if (!mIfNewNodes.empty()) {
        std::vector<NodeRef> newNodes;
        for (auto node : mIfNewNodes)
            newNodes.push_back(NDIfStmt::Create
                    (ref->getCondId()->clone(), ref->getCondN()->clone(), node));
        replace(ref, newNodes);
        mIfNewNodes.clear();
    }
}

void efd::FlattenPass::initImpl() {
    mIfNewNodes.clear();
}

efd::FlattenPass* efd::FlattenPass::Create(QModule* qmod) {
    return new FlattenPass(qmod);
}

