#ifndef __EFD_NODE_VISITOR_H__
#define __EFD_NODE_VISITOR_H__

#include "enfield/Analysis/Nodes.h"

namespace efd {

    /// \brief Interface class to be used as a visitor.
    class NodeVisitor {
        public:
            virtual void visit(NDQasmVersion* v);
            virtual void visit(NDInclude* v);
            virtual void visit(NDDecl* v);
            virtual void visit(NDGateDecl* v);
            virtual void visit(NDOpaque* v);
            virtual void visit(NDQOpMeasure* v);
            virtual void visit(NDQOpReset* v);
            virtual void visit(NDQOpU* v);
            virtual void visit(NDQOpCX* v);
            virtual void visit(NDQOpBarrier* v);
            virtual void visit(NDQOpGeneric* v);
            virtual void visit(NDBinOp* v);
            virtual void visit(NDUnaryOp* v);
            virtual void visit(NDIdRef* v);
            virtual void visit(NDList* v);
            virtual void visit(NDStmtList* v);
            virtual void visit(NDGOpList* v);
            virtual void visit(NDIfStmt* v);
            virtual void visit(NDValue<std::string>* v);
            virtual void visit(NDValue<IntVal>* v);
            virtual void visit(NDValue<RealVal>* v);
    };

};

#endif
