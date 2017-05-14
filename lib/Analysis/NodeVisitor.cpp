#include "enfield/Analysis/NodeVisitor.h"

void efd::NodeVisitor::visit(NDQasmVersion* v) {}
void efd::NodeVisitor::visit(NDInclude* v) {}
void efd::NodeVisitor::visit(NDDecl* v) {}
void efd::NodeVisitor::visit(NDGateDecl* v) {}
void efd::NodeVisitor::visit(NDOpaque* v) {}
void efd::NodeVisitor::visit(NDQOpMeasure* v) {}
void efd::NodeVisitor::visit(NDQOpReset* v) {}
void efd::NodeVisitor::visit(NDQOpU* v) {}
void efd::NodeVisitor::visit(NDQOpCX* v) {}
void efd::NodeVisitor::visit(NDQOpBarrier* v) {}
void efd::NodeVisitor::visit(NDQOpGeneric* v) {}
void efd::NodeVisitor::visit(NDBinOp* v) {}
void efd::NodeVisitor::visit(NDUnaryOp* v) {}
void efd::NodeVisitor::visit(NDIdRef* v) {}
void efd::NodeVisitor::visit(NDList* v) {}
void efd::NodeVisitor::visit(NDStmtList* v) {}
void efd::NodeVisitor::visit(NDGOpList* v) {}
void efd::NodeVisitor::visit(NDIfStmt* v) {}
void efd::NodeVisitor::visit(NDValue<std::string>* v) {}
void efd::NodeVisitor::visit(NDValue<IntVal>* v) {}
void efd::NodeVisitor::visit(NDValue<RealVal>* v) {}
