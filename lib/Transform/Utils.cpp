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
}

// ==--------------- QModulefy ---------------==
namespace {
    class QModulefyVisitor : public efd::NodeVisitor {
        public:
            efd::QModule& mMod;

            QModulefyVisitor(efd::QModule& qmod) : mMod(qmod) {}

            void insertIfNotInsideGate(efd::Node::Ref ref);

            void visit(efd::NDQasmVersion::Ref ref) override;
            void visit(efd::NDInclude::Ref ref) override;
            void visit(efd::NDRegDecl::Ref ref) override;
            void visit(efd::NDGateDecl::Ref ref) override;
            void visit(efd::NDOpaque::Ref ref) override;
            void visit(efd::NDQOpMeasure::Ref ref) override;
            void visit(efd::NDQOpReset::Ref ref) override;
            void visit(efd::NDQOpU::Ref ref) override;
            void visit(efd::NDQOpCX::Ref ref) override;
            void visit(efd::NDQOpBarrier::Ref ref) override;
            void visit(efd::NDQOpGeneric::Ref ref) override;
            void visit(efd::NDIfStmt::Ref ref) override;
    };
}

void QModulefyVisitor::insertIfNotInsideGate(efd::Node::Ref ref) {
    if (efd::instanceOf<efd::NDGOpList>(ref->getParent()))
        return;
    mMod.insertStatementLast(ref->clone());
}

void QModulefyVisitor::visit(efd::NDQasmVersion::Ref ref) {
    auto vNum = efd::uniqueCastForward<efd::NDReal>(ref->getVersion()->clone());
    mMod.setVersion(efd::NDQasmVersion::Create
            (std::move(vNum), efd::NDStmtList::Create()));
}

void QModulefyVisitor::visit(efd::NDInclude::Ref ref) {
    auto fileNode = efd::uniqueCastForward<efd::NDString>(ref->getFilename()->clone());
    mMod.insertInclude(efd::NDInclude::Create
            (std::move(fileNode),
             efd::uniqueCastBackward<efd::Node>(efd::NDStmtList::Create())));
}

void QModulefyVisitor::visit(efd::NDRegDecl::Ref ref) {
    mMod.insertReg(efd::uniqueCastForward<efd::NDRegDecl>(ref->clone()));
}

void QModulefyVisitor::visit(efd::NDGateDecl::Ref ref) {
    mMod.insertGate(efd::uniqueCastForward<efd::NDGateSign>(ref->clone()));
}


void QModulefyVisitor::visit(efd::NDOpaque::Ref ref) {
    mMod.insertGate(efd::uniqueCastForward<efd::NDGateSign>(ref->clone()));
}


void QModulefyVisitor::visit(efd::NDQOpMeasure::Ref ref) {
    insertIfNotInsideGate(ref);
}


void QModulefyVisitor::visit(efd::NDQOpReset::Ref ref) {
    insertIfNotInsideGate(ref);
}


void QModulefyVisitor::visit(efd::NDQOpU::Ref ref) {
    insertIfNotInsideGate(ref);
}


void QModulefyVisitor::visit(efd::NDQOpCX::Ref ref) {
    insertIfNotInsideGate(ref);
}


void QModulefyVisitor::visit(efd::NDQOpBarrier::Ref ref) {
    insertIfNotInsideGate(ref);
}

void QModulefyVisitor::visit(efd::NDQOpGeneric::Ref ref) {
    insertIfNotInsideGate(ref);
}

void efd::ProcessAST(QModule::Ref qmod, Node::Ref root) {
    QModulefyVisitor visitor(*qmod);
    root->apply(&visitor);
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

            void visit(efd::NDQOpU::Ref ref) override;
            void visit(efd::NDQOpCX::Ref ref) override;
            void visit(efd::NDQOpBarrier::Ref ref) override;
            void visit(efd::NDQOpGeneric::Ref ref) override;
            void visit(efd::NDBinOp::Ref ref) override;
            void visit(efd::NDUnaryOp::Ref ref) override;
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

void QArgsReplaceVisitor::visit(efd::NDQOpU::Ref ref) {
    ref->getArgs()->apply(this);
    substituteChildrem(ref->getArgs());
    ref->setQArg(replaceChild(ref->getQArg()));
}

void QArgsReplaceVisitor::visit(efd::NDQOpCX::Ref ref) {
    substituteChildrem(ref);
}

void QArgsReplaceVisitor::visit(efd::NDQOpBarrier::Ref ref) {
    substituteChildrem(ref);
}

void QArgsReplaceVisitor::visit(efd::NDQOpGeneric::Ref ref) {
    ref->getArgs()->apply(this);
    substituteChildrem(ref->getArgs());
    substituteChildrem(ref->getQArgs());
}

void QArgsReplaceVisitor::visit(efd::NDBinOp::Ref ref) {
    ref->getLhs()->apply(this);
    ref->getRhs()->apply(this);
    substituteChildrem(ref);
}

void QArgsReplaceVisitor::visit(efd::NDUnaryOp::Ref ref) {
    substituteChildrem(ref);
}

void efd::InlineGate(QModule::Ref qmod, NDQOpGeneric::Ref qop) {
    std::string gateId = qop->getId()->getVal();
    
    auto gate = qmod->getQGate(gateId);
    assert (!gate->isOpaque() && "Trying to inline opaque gate.");

    NDGateDecl::Ref gateDecl = dynCast<NDGateDecl>(gate);
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
    auto gop = uniqueCastForward<NDGOpList>(gateDecl->getGOpList()->clone());

    // Replacing
    auto it = qmod->findStatement(qop);
    for (auto& op : *gop) {
        op->apply(&visitor);
        it = qmod->insertStatementAfter(it, std::move(op));
    }

    it = qmod->findStatement(qop);
    qmod->removeStatement(it);
}

// ==--------------- Reverse Gate ---------------==
void efd::ReverseCNode(QModule::Ref qmod, Node::Ref node) {
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
                NDQOpGeneric::Ref refGen = dynCast<NDQOpGeneric>(node);
                assert(refGen != nullptr && "Malformed node.");

                NDList::Ref qargs = refGen->getQArgs();
                assert(qargs->getChildNumber() == 2 && "Malformed CNOT call.");

                auto lhs = qargs->getChild(0)->clone();
                auto rhs = qargs->getChild(1)->clone();
                // Swapping the arguments.
                qargs->setChild(0, std::move(rhs));
                qargs->setChild(1, std::move(lhs));

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
        qmod->insertStatementBefore(it, NDQOpGeneric::Create
                (uniqueCastForward<NDId>(H_ID_NODE->clone()),
                 NDList::Create(),
                 uniqueCastForward<NDList>(qArgs->clone())));

        it = parent->findChild(node);
        qmod->insertStatementAfter(it, NDQOpGeneric::Create
                (uniqueCastForward<NDId>(H_ID_NODE->clone()),
                 NDList::Create(),
                 uniqueCastForward<NDList>(qArgs->clone())));
    }
}
