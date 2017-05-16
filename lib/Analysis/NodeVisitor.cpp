#include "enfield/Analysis/NodeVisitor.h"

void efd::NodeVisitor::visit(NDQasmVersion* ref) {}
void efd::NodeVisitor::visit(NDInclude* ref) {}
void efd::NodeVisitor::visit(NDDecl* ref) {}
void efd::NodeVisitor::visit(NDGateDecl* ref) {}
void efd::NodeVisitor::visit(NDOpaque* ref) {}
void efd::NodeVisitor::visit(NDQOpMeasure* ref) {}
void efd::NodeVisitor::visit(NDQOpReset* ref) {}
void efd::NodeVisitor::visit(NDQOpU* ref) {}
void efd::NodeVisitor::visit(NDQOpCX* ref) {}
void efd::NodeVisitor::visit(NDQOpBarrier* ref) {}
void efd::NodeVisitor::visit(NDQOpGeneric* ref) {}
void efd::NodeVisitor::visit(NDBinOp* ref) {}
void efd::NodeVisitor::visit(NDUnaryOp* ref) {}
void efd::NodeVisitor::visit(NDIdRef* ref) {}
void efd::NodeVisitor::visit(NDList* ref) {}
void efd::NodeVisitor::visit(NDStmtList* ref) {}
void efd::NodeVisitor::visit(NDGOpList* ref) {}
void efd::NodeVisitor::visit(NDIfStmt* ref) {}
void efd::NodeVisitor::visit(NDValue<std::string>* ref) {}
void efd::NodeVisitor::visit(NDValue<IntVal>* ref) {}
void efd::NodeVisitor::visit(NDValue<RealVal>* ref) {}

void efd::NodeVisitor::init() {}
