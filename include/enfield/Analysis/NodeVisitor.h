#ifndef __EFD_NODE_VISITOR_H__
#define __EFD_NODE_VISITOR_H__

#include "enfield/Analysis/Nodes.h"

namespace efd {

    /// \brief Interface class to be used as a visitor.
    class NodeVisitor {
        public:
            virtual void visit(NDQasmVersion* v) = 0;
            virtual void visit(NDInclude* v) = 0;
            virtual void visit(NDDecl* v) = 0;
            virtual void visit(NDGateDecl* v) = 0;
            virtual void visit(NDOpaque* v) = 0;
            virtual void visit(NDQOpMeasure* v) = 0;
            virtual void visit(NDQOpReset* v) = 0;
            virtual void visit(NDQOpU* v) = 0;
            virtual void visit(NDQOpCX* v) = 0;
            virtual void visit(NDQOpBarrier* v) = 0;
            virtual void visit(NDQOpGeneric* v) = 0;
            virtual void visit(NDBinOp* v) = 0;
            virtual void visit(NDUnaryOp* v) = 0;
            virtual void visit(NDIdRef* v) = 0;
            virtual void visit(NDList* v) = 0;
            virtual void visit(NDStmtList* v) = 0;
            virtual void visit(NDGOpList* v) = 0;
            virtual void visit(NDIfStmt* v) = 0;
            virtual void visit(NDValue<std::string>* v) = 0;
            virtual void visit(NDValue<IntVal>* v) = 0;
            virtual void visit(NDValue<RealVal>* v) = 0;
    };

};

#endif
