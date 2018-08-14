#include "enfield/Transform/Utils.h"
#include "enfield/Analysis/NodeVisitor.h"
#include "enfield/Analysis/Driver.h"
#include "enfield/Support/RTTI.h"
#include "enfield/Support/uRefCast.h"
#include "enfield/Support/Defs.h"

#include <unordered_map>
#include <iterator>
#include <iostream>

using namespace efd;

// ==--------------- Intrinsic Gates ---------------==
static std::vector<NDGateSign::uRef> IntrinsicGates;
static const std::string IntrinsicGatesStr =
#define EFD_LIB(...) #__VA_ARGS__
#include "enfield/StdLib/intrinsic.inc"
#undef EFD_LIB
;

static void ProcessIntrinsicGates() {
    if (IntrinsicGates.empty()) {
        auto ast = ParseString(IntrinsicGatesStr, false);
        if (!instanceOf<NDStmtList>(ast.get())) {
            ERR << "Intrinsic gates root node of wrong type." << std::endl;
            EFD_ABORT();
        }

        for (auto& gate : *ast) {
            auto gateNode = uniqueCastForward<NDGateSign>(std::move(gate));

            if (gateNode.get() == nullptr) {
                std::string str = (gate == nullptr) ? "nullptr" : gate->toString(false);
                ERR << "Statement is not a gate declaration: `" << str
                    << "`." << std::endl;
                EFD_ABORT();
            }

            IntrinsicGates.push_back(std::move(gateNode));
        }
    }
}

std::vector<NDGateSign::uRef> efd::GetIntrinsicGates() {
    ProcessIntrinsicGates();

    std::vector<NDGateSign::uRef> gates;
    for (auto& gate : IntrinsicGates)
        gates.push_back(uniqueCastForward<NDGateSign>(gate->clone()));

    return gates;
}

namespace efd {
    extern const std::string StdLibCX;

    /// \brief Special node for swap calls.
    struct NDQOpSwap : public NDQOpGen {
        static const std::string IdStr;

        NDQOpSwap(Node::uRef lhs, Node::uRef rhs) :
            NDQOpGen(NDId::Create(IdStr), NDList::Create(), NDList::Create(), K_INTRINSIC_SWAP) {
                auto qargs = getQArgs();
                qargs->addChild(std::move(lhs));
                qargs->addChild(std::move(rhs));
            }
    };

    /// \brief Special node for long cnot calls.
    struct NDQOpLongCX : public NDQOpGen {
        static const std::string IdStr;

        NDQOpLongCX(Node::uRef lhs, Node::uRef middle, Node::uRef rhs) :
            NDQOpGen(NDId::Create(IdStr), NDList::Create(), NDList::Create(), K_INTRINSIC_LCX) {
                auto qargs = getQArgs();
                qargs->addChild(std::move(lhs));
                qargs->addChild(std::move(middle));
                qargs->addChild(std::move(rhs));
            }
    };

    /// \brief Special node for reversal cnot calls.
    struct NDQOpRevCX : public NDQOpGen {
        static const std::string IdStr;

        NDQOpRevCX(Node::uRef lhs, Node::uRef rhs) :
            NDQOpGen(NDId::Create(IdStr), NDList::Create(), NDList::Create(), K_INTRINSIC_REV_CX) {
                auto qargs = getQArgs();
                qargs->addChild(std::move(lhs));
                qargs->addChild(std::move(rhs));
            }
    };
}

const std::string efd::NDQOpSwap::IdStr = "intrinsic_swap__";
const std::string efd::NDQOpLongCX::IdStr = "intrinsic_lcx__";
const std::string efd::NDQOpRevCX::IdStr = "intrinsic_rev_cx__";
const std::string efd::StdLibCX = "cx";

bool efd::IsCNOTGateCall(Node::Ref ref) {
    switch (ref->getKind()) {
        case Node::K_QOP_CX:
            return true;

        case Node::K_QOP_GEN:
            return ref->getOperation() == StdLibCX;

        default:
            break;
    }

    return false;
}

bool efd::IsIntrinsicGateCall(Node::Ref ref) {
    if (auto qopgen = dynCast<NDQOpGen>(ref)) {
        std::string gateid = qopgen->getOperation();

        return gateid == NDQOpSwap::IdStr ||
            gateid == NDQOpLongCX::IdStr ||
            gateid == NDQOpRevCX::IdStr;
    }

    return false;
}

NDQOpGen::IntrinsicKind efd::GetIntrinsicKind(Node::Ref ref) {
    if (!IsIntrinsicGateCall(ref)) {
        std::string str = (ref == nullptr) ? "nullptr" : ref->toString(false);
        ERR << "Node must be an intrinsic gate call: `" << str << "`." << std::endl;
        EFD_ABORT();
    }

    std::string gateid = ref->getOperation();

    NDQOpGen::IntrinsicKind kind;
    if (gateid == efd::NDQOpSwap::IdStr)          kind = NDQOpGen::K_INTRINSIC_SWAP;
    else if (gateid == efd::NDQOpLongCX::IdStr)   kind = NDQOpGen::K_INTRINSIC_LCX;
    else if (gateid == efd::NDQOpRevCX::IdStr)    kind = NDQOpGen::K_INTRINSIC_REV_CX;
    else ERR << "Unreachable." << std::endl;

    return kind;
}

NDQOp::uRef efd::CreateIntrinsicGate(NDQOpGen::IntrinsicKind kind,
                                     std::vector<Node::uRef> qargs) {
    switch (kind) {
        case NDQOpGen::K_INTRINSIC_SWAP:
            if (qargs.size() != 2) {
                ERR << "Intrinsic swap instructions must have 2 qargs. Actual: `"
                    << qargs.size() << "`." << std::endl;
                EFD_ABORT();
            }

            return CreateISwap(std::move(qargs[0]), std::move(qargs[1]));

        case NDQOpGen::K_INTRINSIC_REV_CX:
            if (qargs.size() != 2) {
                ERR << "Intrinsic reverse-cnot instructions must 2 qargs. Actual:"
                    << qargs.size() << "`." << std::endl;
                EFD_ABORT();
            }

            return CreateIRevCX(std::move(qargs[0]), std::move(qargs[1]));

        case NDQOpGen::K_INTRINSIC_LCX:
            if (qargs.size() != 3) {
                ERR << "Intrinsic long-cnot instructions must have 3 qargs. Actual:"
                    << qargs.size() << "`." << std::endl;
                EFD_ABORT();
            }

            return CreateILongCX(std::move(qargs[0]), std::move(qargs[1]), std::move(qargs[2]));

        default:
            EFD_ABORT();
    }
}

NDQOp::uRef efd::CreateISwap(Node::uRef lhs, Node::uRef rhs) {
    auto node = new NDQOpSwap(std::move(lhs), std::move(rhs));
    return NDQOp::uRef(node);
}

NDQOp::uRef efd::CreateILongCX
(Node::uRef lhs, Node::uRef middle, Node::uRef rhs) {
    auto node = new NDQOpLongCX(std::move(lhs), std::move(middle), std::move(rhs));
    return NDQOp::uRef(node);
}

NDQOp::uRef efd::CreateIRevCX(Node::uRef lhs, Node::uRef rhs) {
    auto node = new NDQOpRevCX(std::move(lhs), std::move(rhs));
    return NDQOp::uRef(node);
}

// ==--------------- QModulefy ---------------==
namespace efd {
    class QModulefyVisitor : public NodeVisitor {
        private:
            Node::uRef getClonedOrIntrinsic(Node::Ref ref);

        public:
            QModule& mMod;

            NDGateDecl::Ref mCurGate;
            NDInclude::Ref mCurIncl;

            QModulefyVisitor(QModule& qmod)
                : mMod(qmod), mCurGate(nullptr), mCurIncl(nullptr) {}

            void visit(NDQasmVersion::Ref ref) override;
            void visit(NDInclude::Ref ref) override;
            void visit(NDRegDecl::Ref ref) override;
            void visit(NDGateDecl::Ref ref) override;
            void visit(NDOpaque::Ref ref) override;
            void visit(NDQOpMeasure::Ref ref) override;
            void visit(NDQOpReset::Ref ref) override;
            void visit(NDQOpU::Ref ref) override;
            void visit(NDQOpCX::Ref ref) override;
            void visit(NDQOpBarrier::Ref ref) override;
            void visit(NDQOpGen::Ref ref) override;
            void visit(NDIfStmt::Ref ref) override;
            void visit(NDStmtList::Ref ref) override;
    };
}

efd::Node::uRef efd::QModulefyVisitor::getClonedOrIntrinsic(Node::Ref ref) {
    auto cloned = ref->clone();
    auto ifstmt = dynCast<NDIfStmt>(cloned.get());
    auto qop = dynCast<NDQOp>(cloned.get());

    if (qop == nullptr && ifstmt) {
        qop = ifstmt->getQOp();
    }

    if (qop == nullptr) {
        std::string str = (ref == nullptr) ? "nullptr" : ref->toString(false);
        ERR << "Node is neither NDQOp nor NDIfStmt. Actual: `" << str << "`." << std::endl;
        EFD_ABORT();
    }

    NDQOp::uRef intrinsic(nullptr);
    if (IsIntrinsicGateCall(qop)) {
        auto kind = GetIntrinsicKind(qop);
        auto qargs = qop->getQArgs();

        std::vector<Node::uRef> qargsVector;
        for (auto& qarg : *qargs) { qargsVector.push_back(qarg->clone()); }
        intrinsic = CreateIntrinsicGate(kind, std::move(qargsVector));
    }

    if (intrinsic.get() != nullptr) {
        if (ifstmt != nullptr) {
            ifstmt->setQOp(std::move(intrinsic));
        } else {
            cloned = std::move(intrinsic);
        }
    }

    return cloned;
}

void efd::QModulefyVisitor::visit(NDQasmVersion::Ref ref) {
    if (mCurIncl != nullptr) {
        auto vNum = uniqueCastForward<NDReal>(ref->getVersion()->clone());
        mMod.setVersion(NDQasmVersion::Create(std::move(vNum), NDStmtList::Create()));
    }

    visitChildren(ref);
}

void efd::QModulefyVisitor::visit(NDInclude::Ref ref) {
    auto fileNode = uniqueCastForward<NDString>(ref->getFilename()->clone());
    mMod.insertInclude(NDInclude::Create
            (std::move(fileNode), uniqueCastBackward<Node>(NDStmtList::Create())));

    mCurIncl = ref;
    visitChildren(ref);
    mCurIncl = nullptr;
}

void efd::QModulefyVisitor::visit(NDRegDecl::Ref ref) {
    mMod.insertReg(uniqueCastForward<NDRegDecl>(ref->clone()));
}

void efd::QModulefyVisitor::visit(NDGateDecl::Ref ref) {
    auto clone = uniqueCastForward<NDGateSign>(ref->clone());

    if (mCurIncl != nullptr) {
        clone->setInInclude();
    }

    mMod.insertGate(std::move(clone));
}

void efd::QModulefyVisitor::visit(NDOpaque::Ref ref) {
    mMod.insertGate(uniqueCastForward<NDGateSign>(ref->clone()));
}


void efd::QModulefyVisitor::visit(NDQOpMeasure::Ref ref) {
    mMod.insertStatementLast(ref->clone());
}


void efd::QModulefyVisitor::visit(NDQOpReset::Ref ref) {
    mMod.insertStatementLast(ref->clone());
}


void efd::QModulefyVisitor::visit(NDQOpU::Ref ref) {
    mMod.insertStatementLast(ref->clone());
}


void efd::QModulefyVisitor::visit(NDQOpCX::Ref ref) {
    mMod.insertStatementLast(ref->clone());
}

void efd::QModulefyVisitor::visit(NDQOpBarrier::Ref ref) {
    mMod.insertStatementLast(ref->clone());
}

// Both NDQOpGen and NDQOpIfStmt can be (the first) or have (the latter) an intrinsic gate.
void efd::QModulefyVisitor::visit(NDQOpGen::Ref ref) {
    auto cloned = getClonedOrIntrinsic(ref);
    mMod.insertStatementLast(std::move(cloned));
}

void efd::QModulefyVisitor::visit(NDIfStmt::Ref ref) {
    auto cloned = getClonedOrIntrinsic(ref);
    mMod.insertStatementLast(std::move(cloned));
}

void efd::QModulefyVisitor::visit(NDStmtList::Ref ref) {
    visitChildren(ref);
}

void efd::ProcessAST(QModule::Ref qmod, Node::Ref root) {
    QModulefyVisitor visitor(*qmod);
    root->apply(&visitor);
}

// ==--------------- Inlining ---------------==
static void QArgsReplaceVisitorVisit(NodeVisitor* visitor, NDQOp::Ref ref);

typedef std::unordered_map<std::string, Node::Ref> VarMap;

namespace efd {
    class QArgsReplaceVisitor : public NodeVisitor {
        public:
            VarMap& varMap;

            QArgsReplaceVisitor(VarMap& varMap) : varMap(varMap) {}

            void substituteChildrem(Node::Ref ref);
            Node::uRef replaceChild(Node::Ref ref);

            void visit(NDQOpU::Ref ref) override;
            void visit(NDQOpCX::Ref ref) override;
            void visit(NDList::Ref ref) override;
            void visit(NDQOpBarrier::Ref ref) override;
            void visit(NDQOpGen::Ref ref) override;
            void visit(NDBinOp::Ref ref) override;
            void visit(NDUnaryOp::Ref ref) override;
            void visit(NDIfStmt::Ref ref) override;
    };
}

Node::uRef efd::QArgsReplaceVisitor::replaceChild(Node::Ref child) {
    std::string _id = child->toString();

    if (varMap.find(_id) != varMap.end()) {
        return varMap[_id]->clone();
    }

    return Node::uRef(nullptr);
}

void efd::QArgsReplaceVisitor::substituteChildrem(Node::Ref ref) {
    for (uint32_t i = 0, e = ref->getChildNumber(); i < e; ++i) {
        auto newChild = replaceChild(ref->getChild(i));
        if (newChild.get() != nullptr) {
            ref->setChild(i, std::move(newChild));
        }
    }
}

void efd::QArgsReplaceVisitor::visit(NDList::Ref ref) {
    visitChildren(ref);
    substituteChildrem(ref);
}

void efd::QArgsReplaceVisitor::visit(NDQOpU::Ref ref) {
    QArgsReplaceVisitorVisit(this, (NDQOp::Ref) ref);
}

void efd::QArgsReplaceVisitor::visit(NDQOpCX::Ref ref) {
    QArgsReplaceVisitorVisit(this, (NDQOp::Ref) ref);
}

void efd::QArgsReplaceVisitor::visit(NDQOpBarrier::Ref ref) {
    QArgsReplaceVisitorVisit(this, (NDQOp::Ref) ref);
}

void efd::QArgsReplaceVisitor::visit(NDQOpGen::Ref ref) {
    QArgsReplaceVisitorVisit(this, (NDQOp::Ref) ref);
}

void efd::QArgsReplaceVisitor::visit(NDBinOp::Ref ref) {
    ref->getLhs()->apply(this);
    ref->getRhs()->apply(this);
    substituteChildrem(ref);
}

void efd::QArgsReplaceVisitor::visit(NDUnaryOp::Ref ref) {
    visitChildren(ref);
    substituteChildrem(ref);
}

void efd::QArgsReplaceVisitor::visit(NDIfStmt::Ref ref) {
    // We only need to visit the qop.
    ref->getQOp()->apply(this);
}

void efd::InlineGate(QModule::Ref qmod, NDQOp::Ref qop) {
    std::string gateId = qop->getId()->getVal();
    
    auto gate = qmod->getQGate(gateId);
    if (gate->isOpaque()) {
        ERR << "Trying to inline opaque gate: `" << gateId << "`." << std::endl;
        EFD_ABORT();
    }

    NDGateDecl::Ref gateDecl = dynCast<NDGateDecl>(gate);
    if (gateDecl == nullptr) {
        ERR << "No gate with such id found: `" << gateId << "`." << std::endl;
        EFD_ABORT();
    }

    VarMap varMap;

    Node::Ref gateQArgs = gateDecl->getQArgs();
    Node::Ref qopQArgs = qop->getQArgs();
    for (uint32_t i = 0, e = gateQArgs->getChildNumber(); i < e; ++i)
        varMap[gateQArgs->getChild(i)->toString()] = qopQArgs->getChild(i);
    
    Node::Ref gateArgs = gateDecl->getArgs();
    Node::Ref qopArgs = qop->getArgs();
    for (uint32_t i = 0, e = gateArgs->getChildNumber(); i < e; ++i)
        varMap[gateArgs->getChild(i)->toString()] = qopArgs->getChild(i);

    // ------ Replacing
    QArgsReplaceVisitor visitor(varMap);
    auto gop = uniqueCastForward<NDGOpList>(gateDecl->getGOpList()->clone());

    // 'stmt' is the node we are going to replace.
    Node::Ref stmt = nullptr;
    auto ifstmt = dynCast<NDIfStmt>(qop->getParent());
    if (ifstmt != nullptr) stmt = ifstmt;
    else stmt = qop;

    // Replace the arguments.
    std::vector<Node::uRef> inlinedNodes;
    for (auto& op : *gop) {
        auto qop = std::move(op);
        // If its parent is an NDIfStmt, we wrap the the operation into
        // a clone of the if.
        if (ifstmt != nullptr) {
            auto ifclone = uniqueCastForward<NDIfStmt>(ifstmt->clone());
            ifclone->setQOp(uniqueCastForward<NDQOp>(std::move(qop)));
            qop.reset(ifclone.release());
        }

        // The 'visitor' is applied only in the 'qop'.
        qop->apply(&visitor);
        inlinedNodes.push_back(std::move(qop));
    }

    qmod->replaceStatement(stmt, std::move(inlinedNodes));
}

static void QArgsReplaceVisitorVisit(NodeVisitor* visitor, NDQOp::Ref ref) {
    ref->getArgs()->apply(visitor);
    ref->getQArgs()->apply(visitor);
}
