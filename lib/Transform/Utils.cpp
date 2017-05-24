#include "enfield/Transform/Utils.h"
#include "enfield/Analysis/NodeVisitor.h"
#include "enfield/Support/RTTI.h"

#include <cassert>
#include <unordered_map>
#include <iterator>

namespace efd {
    const NodeRef SWAP_ID_NODE = efd::NDId::Create("__swap__"); 
    const NodeRef H_ID_NODE = efd::NDId::Create("h"); 

    /// \brief Enum that indicates where to place a instruction.
    enum Loc {
        LOC_BEFORE, LOC_AFTER
    };

    /// \brief Inserts the node wherever \where is indicating.
    void InsertNode(Node::Iterator& it, NodeRef node, Loc where);
    /// \brief Inserts a swap wherever \where is indicating.
    void InsertSwap(NodeRef prev, NodeRef lhs, NodeRef rhs, Loc where);
}

void efd::InsertNode(Node::Iterator& it, NodeRef node, Loc where) {
    NDList* parent = dynCast<NDList>((*it)->getParent());
    assert(parent != nullptr && "Parent node must be of type NDList.");
    if (where == LOC_AFTER) ++it;
    parent->addChild(it, node);
}

void efd::InsertNodeAfter(Node::Iterator& it, NodeRef node) {
    efd::InsertNode(it, node, LOC_AFTER);
}

void efd::InsertNodeBefore(Node::Iterator& it, NodeRef node) {
    efd::InsertNode(it, node, LOC_BEFORE);
}

// ==--------------- Inlining ---------------==
typedef std::unordered_map<std::string, efd::NodeRef> VarMap;

namespace {
    class QArgsReplaceVisitor : public efd::NodeVisitor {
        public:
            VarMap& varMap;

            QArgsReplaceVisitor(VarMap& varMap) : varMap(varMap) {}

            void substituteChildrem(efd::NodeRef ref);

            void visit(efd::NDQOpU* ref) override;
            void visit(efd::NDQOpCX* ref) override;
            void visit(efd::NDQOpBarrier* ref) override;
            void visit(efd::NDQOpGeneric* ref) override;
            void visit(efd::NDBinOp* ref) override;
            void visit(efd::NDUnaryOp* ref) override;
    };
}

void QArgsReplaceVisitor::substituteChildrem(efd::NodeRef ref) {
    for (unsigned i = 0, e = ref->getChildNumber(); i < e; ++i) {
        efd::NodeRef child = ref->getChild(i);
        std::string _id = child->toString();

        if (varMap.find(_id) != varMap.end()) {
            ref->setChild(i, varMap[_id]);
        }
    }
}

void QArgsReplaceVisitor::visit(efd::NDQOpU* ref) {
    ref->getArgs()->apply(this);
    substituteChildrem(ref->getArgs());
    substituteChildrem(ref->getQArg());
}

void QArgsReplaceVisitor::visit(efd::NDQOpCX* ref) {
    substituteChildrem(ref);
}

void QArgsReplaceVisitor::visit(efd::NDQOpBarrier* ref) {
    substituteChildrem(ref);
}

void QArgsReplaceVisitor::visit(efd::NDQOpGeneric* ref) {
    ref->getArgs()->apply(this);
    substituteChildrem(ref->getArgs());
    substituteChildrem(ref->getQArgs());
}

void QArgsReplaceVisitor::visit(efd::NDBinOp* ref) {
    ref->getLhs()->apply(this);
    ref->getRhs()->apply(this);
    substituteChildrem(ref);
}

void QArgsReplaceVisitor::visit(efd::NDUnaryOp* ref) {
    substituteChildrem(ref);
}

void efd::InlineGate(QModule* qmod, NDQOpGeneric* qop) {
    std::string gateId = qop->getId()->getVal();
    
    NDGateDecl* gateDecl = qmod->getQGate(gateId);
    assert(gateDecl != nullptr && "No gate with such id found.");


    VarMap varMap;

    NodeRef gateQArgs = gateDecl->getQArgs();
    NodeRef qopQArgs = qop->getQArgs();
    for (unsigned i = 0, e = gateQArgs->getChildNumber(); i < e; ++i)
        varMap[gateQArgs->getChild(i)->toString()] = qopQArgs->getChild(i);
    
    NodeRef gateArgs = gateDecl->getArgs();
    NodeRef qopArgs = qop->getArgs();
    for (unsigned i = 0, e = gateArgs->getChildNumber(); i < e; ++i)
        varMap[gateArgs->getChild(i)->toString()] = qopArgs->getChild(i);

    QArgsReplaceVisitor visitor(varMap);
    std::vector<NodeRef> opList = CloneGOp(gateDecl);
    for (auto op : opList) {
        op->apply(&visitor);
    }

    ReplaceNodes(qop, opList);
}

std::vector<efd::NodeRef> efd::CloneGOp(NDGateDecl* gateDecl) {
    std::vector<NodeRef> cloned;

    NDGOpList* gopList = gateDecl->getGOpList();
    for (auto op : *gopList) {
        cloned.push_back(op->clone());
    }

    return cloned;
}

void efd::ReplaceNodes(NodeRef ref, std::vector<NodeRef> nodes) {
    unsigned dist;
    Node::Iterator it;

    if (NDList* parent = dynCast<NDList>(ref->getParent())) {

        it = parent->findChild(ref);
        dist = std::distance(parent->begin(), it);
        for (auto child : nodes)
            efd::InsertNodeAfter(it, child);

        auto old = parent->begin() + dist;
        parent->removeChild(old);

    } else if (NDIfStmt* parent = dynCast<NDIfStmt>(ref->getParent())) {
        NDList* ifParent = dynCast<NDList>(parent->getParent());
        assert(ifParent != nullptr && "The parent of an If node has to be a NDList.");

        it = ifParent->findChild(parent);
        dist = std::distance(ifParent->begin(), it);
        for (auto child : nodes)
            efd::InsertNodeAfter(it, NDIfStmt::Create(parent->getCondId()->clone(), 
                        parent->getCondN()->clone(), child));

        auto old = ifParent->begin() + dist;
        ifParent->removeChild(parent);

    } else {
        assert(false && "Unreacheable. Node must be either If or List.");
    }
}

// ==--------------- InsertSwap ---------------==
void efd::InsertSwap(NodeRef prev, NodeRef lhs, NodeRef rhs, Loc where) {
    NodeRef baseParent = prev->getParent();

    NDList* qArgs = dynCast<NDList>(NDList::Create());
    qArgs->addChild(lhs->clone());
    qArgs->addChild(rhs->clone());
    NodeRef callNode = NDQOpGeneric::Create(SWAP_ID_NODE->clone(), NDList::Create(), qArgs);

    if (NDList* parent = dynCast<NDList>(baseParent)) {
        auto it = parent->findChild(prev);
        if (where == LOC_AFTER)
            efd::InsertNodeAfter(it, callNode);
        else if (where == LOC_BEFORE)
            efd::InsertNodeBefore(it, callNode);

    } else if (NDIfStmt* parent = dynCast<NDIfStmt>(baseParent)) {
        NDList* ifParent = dynCast<NDList>(parent->getParent());
        assert(ifParent != nullptr && "The parent of an If node has to be a NDList.");
        auto it = ifParent->findChild(prev);

        // In this case, the swap must be global, and not in the if scope.
        if (where == LOC_AFTER)
            efd::InsertNodeAfter(it, callNode);
        else if (where == LOC_BEFORE)
            efd::InsertNodeBefore(it, callNode);

    } else {
        assert(false && "Unreacheable. Node must be either If or List.");
    }
}

void efd::InsertSwapAfter(NodeRef prev, NodeRef lhs, NodeRef rhs) {
    efd::InsertSwap(prev, lhs, rhs, LOC_AFTER);
}

void efd::InsertSwapBefore(NodeRef prev, NodeRef lhs, NodeRef rhs) {
    efd::InsertSwap(prev, lhs, rhs, LOC_BEFORE);
}

// ==--------------- Reverse Gate ---------------==
void efd::ReverseCNode(NodeRef node) {
    std::vector<NodeRef> qArgs;

    switch (node->getKind()) {
        case Node::K_QOP_CX:
            {
                NDQOpCX* refCX = dynCast<NDQOpCX>(node);
                assert(refCX != nullptr && "Malformed node.");
                qArgs.push_back(refCX->getLhs());
                qArgs.push_back(refCX->getRhs());
            }
            break;

        case Node::K_QOP_GENERIC:
            {
                NDQOpGeneric* refGen = dynCast<NDQOpGeneric>(node);
                assert(refGen != nullptr && "Malformed node.");
                for (auto child : *refGen->getQArgs())
                    qArgs.push_back(child);
            }
            break;

        default:
            assert(false && "Can't reverse any other node, but CX and QOpGeneric.");
    }

    NodeRef parent = node->getParent();
    Node::Iterator it = parent->findChild(node), oldIt = it;
    for (auto qbit : qArgs) {
        NDList* qArgs = dynCast<NDList>(NDList::Create());
        qArgs->addChild(qbit);

        efd::InsertNodeBefore(it, NDQOpGeneric::Create(H_ID_NODE->clone(), 
                    NDList::Create(), qArgs->clone()));
        it = oldIt;

        efd::InsertNodeAfter(it, NDQOpGeneric::Create(H_ID_NODE->clone(), 
                    NDList::Create(), qArgs->clone()));
        it = oldIt;
    }
}
