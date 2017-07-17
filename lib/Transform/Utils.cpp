#include "enfield/Transform/Utils.h"
#include "enfield/Analysis/NodeVisitor.h"
#include "enfield/Support/RTTI.h"
#include "enfield/Support/uRefCast.h"

#include <cassert>
#include <unordered_map>
#include <iterator>
#include <iostream>

namespace efd {
    NDId::uRef SWAP_ID_NODE(efd::NDId::Create("__swap__"));
    NDId::uRef H_ID_NODE(efd::NDId::Create("h")); 
    NDId::uRef CX_ID_NODE(efd::NDId::Create("cx")); 

    /// \brief Enum that indicates where to place a instruction.
    enum Loc {
        LOC_BEFORE, LOC_AFTER
    };

    /// \brief Inserts the node wherever \where is indicating.
    void InsertNode(Node::Iterator& it, Node::uRef node, Loc where);
    /// \brief Inserts a swap wherever \where is indicating.
    void InsertSwap(Node::Ref prev, Node::Ref lhs, Node::Ref rhs, Loc where);
}

void efd::InsertNode(Node::Iterator& it, Node::uRef node, Loc where) {
    NDList::Ref parent = dynCast<NDList>((*it)->getParent());
    assert(parent != nullptr && "Parent node must be of type NDList.");
    if (where == LOC_AFTER) ++it;
    parent->addChild(it, std::move(node));
}

void efd::InsertNodeAfter(Node::Iterator& it, Node::uRef node) {
    efd::InsertNode(it, std::move(node), LOC_AFTER);
}

void efd::InsertNodeBefore(Node::Iterator& it, Node::uRef node) {
    efd::InsertNode(it, std::move(node), LOC_BEFORE);
}

// ==--------------- Inlining ---------------==
typedef std::unordered_map<std::string, efd::Node::Ref> VarMap;

namespace {
    class QArgsReplaceVisitor : public efd::NodeVisitor {
        public:
            VarMap& varMap;

            QArgsReplaceVisitor(VarMap& varMap) : varMap(varMap) {}

            void substituteChildrem(efd::Node::Ref ref);
            efd::Node::uRef replaceChild(efd::Node::Ref ref);

            void visit(efd::NDQOpU* ref) override;
            void visit(efd::NDQOpCX* ref) override;
            void visit(efd::NDQOpBarrier* ref) override;
            void visit(efd::NDQOpGeneric* ref) override;
            void visit(efd::NDBinOp* ref) override;
            void visit(efd::NDUnaryOp* ref) override;
    };
}

efd::Node::uRef QArgsReplaceVisitor::replaceChild(efd::Node::Ref child) {
    std::string _id = child->toString();

    if (varMap.find(_id) != varMap.end()) {
        return varMap[_id]->clone();
    }

    return efd::Node::uRef(nullptr);
}

void QArgsReplaceVisitor::substituteChildrem(efd::Node::Ref ref) {
    for (unsigned i = 0, e = ref->getChildNumber(); i < e; ++i) {
        auto newChild = replaceChild(ref->getChild(i));
        if (newChild.get() != nullptr) {
            ref->setChild(i, std::move(newChild));
        }
    }
}

void QArgsReplaceVisitor::visit(efd::NDQOpU* ref) {
    ref->getArgs()->apply(this);
    substituteChildrem(ref->getArgs());
    ref->setQArg(replaceChild(ref->getQArg()));
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

    Node::Ref gateQArgs = gateDecl->getQArgs();
    Node::Ref qopQArgs = qop->getQArgs();
    for (unsigned i = 0, e = gateQArgs->getChildNumber(); i < e; ++i)
        varMap[gateQArgs->getChild(i)->toString()] = qopQArgs->getChild(i);
    
    Node::Ref gateArgs = gateDecl->getArgs();
    Node::Ref qopArgs = qop->getArgs();
    for (unsigned i = 0, e = gateArgs->getChildNumber(); i < e; ++i)
        varMap[gateArgs->getChild(i)->toString()] = qopArgs->getChild(i);

    QArgsReplaceVisitor visitor(varMap);
    std::vector<Node::sRef> opList = CloneGOp(gateDecl);
    for (auto op : opList) {
        op->apply(&visitor);
    }

    ReplaceNodes(qop, opList);
}

std::vector<efd::Node::sRef> efd::CloneGOp(NDGateDecl* gateDecl) {
    std::vector<Node::sRef> cloned;

    NDGOpList::Ref gopList = gateDecl->getGOpList();
    for (auto& op : *gopList) {
        cloned.push_back(Node::sRef(op->clone().release()));
    }

    return cloned;
}

void efd::ReplaceNodes(Node::Ref ref, std::vector<Node::sRef> nodes) {
    unsigned dist;
    Node::Iterator it;

    if (NDList* parent = dynCast<NDList>(ref->getParent())) {

        it = parent->findChild(ref);
        assert(it != parent->end() && "Node removed from parent.");

        dist = std::distance(parent->begin(), it);
        for (auto child : nodes)
            efd::InsertNodeAfter(it, child->clone());
        auto old = parent->begin() + dist;
        parent->removeChild(old);

    } else if (NDIfStmt* parent = dynCast<NDIfStmt>(ref->getParent())) {
        NDList* ifParent = dynCast<NDList>(parent->getParent());
        assert(ifParent != nullptr && "The parent of an If node has to be a NDList.");

        it = ifParent->findChild(parent);
        assert(it != parent->end() && "Node removed from parent.");

        dist = std::distance(ifParent->begin(), it);
        for (auto child : nodes)
            efd::InsertNodeAfter(it, NDIfStmt::Create
                    (uniqueCastForward<NDId>(parent->getCondId()->clone()),
                     uniqueCastForward<NDInt>(parent->getCondN()->clone()),
                     child->clone()));

        auto old = ifParent->begin() + dist;
        ifParent->removeChild(parent);

    } else {
        assert(false && "Unreacheable. Node must be either If or List.");
    }
}

// ==--------------- InsertSwap ---------------==
void efd::InsertSwap(Node::Ref prev, Node::Ref lhs, Node::Ref rhs, Loc where) {
    Node::Ref baseParent = prev->getParent();

    auto qArgs = NDList::Create();
    qArgs->addChild(lhs->clone());
    qArgs->addChild(rhs->clone());

    // Creating swap node, and setting the generated property.
    auto callNode = NDQOpGeneric::Create
        (uniqueCastBackward<NDId>(SWAP_ID_NODE->clone()), NDList::Create(), std::move(qArgs));
    callNode->setGenerated();

    if (NDList* parent = dynCast<NDList>(baseParent)) {
        auto it = parent->findChild(prev);
        if (where == LOC_AFTER)
            efd::InsertNodeAfter(it, std::move(callNode));
        else if (where == LOC_BEFORE)
            efd::InsertNodeBefore(it, std::move(callNode));

    } else if (NDIfStmt* parent = dynCast<NDIfStmt>(baseParent)) {
        NDList* ifParent = dynCast<NDList>(parent->getParent());
        assert(ifParent != nullptr && "The parent of an If node has to be a NDList.");
        auto it = ifParent->findChild(prev);

        // In this case, the swap must be global, and not in the if scope.
        if (where == LOC_AFTER)
            efd::InsertNodeAfter(it, std::move(callNode));
        else if (where == LOC_BEFORE)
            efd::InsertNodeBefore(it, std::move(callNode));

    } else {
        assert(false && "Unreacheable. Node must be either If or List.");
    }
}

void efd::InsertSwapAfter(Node::Ref prev, Node::Ref lhs, Node::Ref rhs) {
    efd::InsertSwap(prev, lhs, rhs, LOC_AFTER);
}

void efd::InsertSwapBefore(Node::Ref prev, Node::Ref lhs, Node::Ref rhs) {
    efd::InsertSwap(prev, lhs, rhs, LOC_BEFORE);
}

// ==--------------- Reverse Gate ---------------==
void efd::ReverseCNode(Node::Ref node) {
    std::vector<Node::Ref> qArgs;

    switch (node->getKind()) {
        case Node::K_QOP_CX:
            {
                NDQOpCX::Ref refCX = dynCast<NDQOpCX>(node);
                assert(refCX != nullptr && "Malformed node.");

                Node::Ref lhs = refCX->getLhs();
                Node::Ref rhs = refCX->getRhs();
                // Swapping the arguments.
                refCX->setLhs(rhs->clone());
                refCX->setRhs(lhs->clone());

                qArgs.push_back(refCX->getLhs());
                qArgs.push_back(refCX->getRhs());
            }
            break;

        case Node::K_QOP_GENERIC:
            {
                NDQOpGeneric* refGen = dynCast<NDQOpGeneric>(node);
                assert(refGen != nullptr && "Malformed node.");

                Node::Ref qargs = refGen->getQArgs();
                assert(qargs->getChildNumber() == 2 && "Malformed CNOT call.");

                Node::Ref lhs = qargs->getChild(0);
                Node::Ref rhs = qargs->getChild(1);
                // Swapping the arguments.
                qargs->setChild(0, rhs->clone());
                qargs->setChild(1, lhs->clone());

                qArgs.push_back(qargs->getChild(0));
                qArgs.push_back(qargs->getChild(1));
            }
            break;

        default:
            assert(false && "Can't reverse any other node, but CX and QOpGeneric.");
    }

    Node::Ref parent = node->getParent();
    Node::Iterator it;
    for (auto qbit : qArgs) {
        auto qArgs = NDList::Create();
        qArgs->addChild(qbit->clone());

        it = parent->findChild(node);
        efd::InsertNodeBefore(it, NDQOpGeneric::Create
                (uniqueCastForward<NDId>(H_ID_NODE->clone()),
                 NDList::Create(),
                 uniqueCastForward<NDList>(qArgs->clone())));

        it = parent->findChild(node);
        efd::InsertNodeAfter(it, NDQOpGeneric::Create
                (uniqueCastForward<NDId>(H_ID_NODE->clone()),
                 NDList::Create(),
                 uniqueCastForward<NDList>(qArgs->clone())));
    }
}
