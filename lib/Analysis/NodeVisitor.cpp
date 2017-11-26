#include "enfield/Analysis/NodeVisitor.h"

void efd::NodeVisitor::visit(NDQasmVersion::Ref ref) {}
void efd::NodeVisitor::visit(NDInclude::Ref ref) {}
void efd::NodeVisitor::visit(NDRegDecl::Ref ref) {}
void efd::NodeVisitor::visit(NDGateDecl::Ref ref) {}
void efd::NodeVisitor::visit(NDOpaque::Ref ref) {}
void efd::NodeVisitor::visit(NDQOpMeasure::Ref ref) {}
void efd::NodeVisitor::visit(NDQOpReset::Ref ref) {}
void efd::NodeVisitor::visit(NDQOpU::Ref ref) {}
void efd::NodeVisitor::visit(NDQOpCX::Ref ref) {}
void efd::NodeVisitor::visit(NDQOpBarrier::Ref ref) {}
void efd::NodeVisitor::visit(NDQOpGen::Ref ref) {}
void efd::NodeVisitor::visit(NDBinOp::Ref ref) {}
void efd::NodeVisitor::visit(NDUnaryOp::Ref ref) {}
void efd::NodeVisitor::visit(NDIdRef::Ref ref) {}
void efd::NodeVisitor::visit(NDList::Ref ref) {}
void efd::NodeVisitor::visit(NDStmtList::Ref ref) {}
void efd::NodeVisitor::visit(NDGOpList::Ref ref) {}
void efd::NodeVisitor::visit(NDIfStmt::Ref ref) {}
void efd::NodeVisitor::visit(NDValue<std::string>::Ref ref) {}
void efd::NodeVisitor::visit(NDValue<IntVal>::Ref ref) {}
void efd::NodeVisitor::visit(NDValue<RealVal>::Ref ref) {}

void efd::NodeVisitor::visitChildren(Node::Ref ref) {
    for (auto& child : *ref) {
        child->apply(this);
    }
}
