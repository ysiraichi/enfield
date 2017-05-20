#ifndef __EFD_NODE_VISITOR_H__
#define __EFD_NODE_VISITOR_H__

#include "enfield/Analysis/Nodes.h"

namespace efd {

    /// \brief Interface class to be used as a visitor.
    class NodeVisitor {
        public:
            virtual void visit(NDQasmVersion* ref);
            virtual void visit(NDInclude* ref);
            virtual void visit(NDDecl* ref);
            virtual void visit(NDGateDecl* ref);
            virtual void visit(NDOpaque* ref);
            virtual void visit(NDQOpMeasure* ref);
            virtual void visit(NDQOpReset* ref);
            virtual void visit(NDQOpU* ref);
            virtual void visit(NDQOpCX* ref);
            virtual void visit(NDQOpBarrier* ref);
            virtual void visit(NDQOpGeneric* ref);
            virtual void visit(NDBinOp* ref);
            virtual void visit(NDUnaryOp* ref);
            virtual void visit(NDIdRef* ref);
            virtual void visit(NDList* ref);
            virtual void visit(NDStmtList* ref);
            virtual void visit(NDGOpList* ref);
            virtual void visit(NDIfStmt* ref);
            virtual void visit(NDValue<std::string>* ref);
            virtual void visit(NDValue<IntVal>* ref);
            virtual void visit(NDValue<RealVal>* ref);

            /// \brief Method for initializing the class before
            /// every run.
            virtual void init();
    };

};

#endif
