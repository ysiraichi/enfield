#include "enfield/Transform/GateInline.h"
#include "enfield/Analysis/NodeVisitor.h"
#include "enfield/Support/RTTI.h"

#include <cassert>
#include <unordered_map>

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
    if (NDList* parent = dynCast<NDList>(ref->getParent())) {
        auto It = parent->findChild(ref);
        for (auto child : nodes) {
            parent->addChild(It, child);
            ++It;
        }
        parent->removeChild(ref);
    } else if (NDIfStmt* parent = dynCast<NDIfStmt>(ref->getParent())) {
        NDList* ifParent = dynCast<NDList>(parent->getParent());
        assert(ifParent != nullptr && "The parent of an If node has to be a NDList.");

        auto It = ifParent->findChild(parent);
        for (auto child : nodes) {
            ifParent->addChild(It, NDIfStmt::Create(parent->getCondId()->clone(), 
                        parent->getCondN()->clone(), child));
            ++It;
        }
        ifParent->removeChild(parent);

    } else {
        assert(false && "Unreacheable. Node must be either If or List.");
    }
}
