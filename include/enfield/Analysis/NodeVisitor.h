#ifndef __EFD_NODE_VISITOR_H__
#define __EFD_NODE_VISITOR_H__

#include "enfield/Analysis/Nodes.h"

namespace efd {

    /// \brief Interface class to be used as a visitor.
    class NodeVisitor {
        public:
            typedef NodeVisitor* Ref;

            virtual void visit(NDQasmVersion::Ref ref);
            virtual void visit(NDInclude::Ref ref);
            virtual void visit(NDRegDecl::Ref ref);
            virtual void visit(NDGateDecl::Ref ref);
            virtual void visit(NDOpaque::Ref ref);
            virtual void visit(NDQOpMeasure::Ref ref);
            virtual void visit(NDQOpReset::Ref ref);
            virtual void visit(NDQOpU::Ref ref);
            virtual void visit(NDQOpCX::Ref ref);
            virtual void visit(NDQOpBarrier::Ref ref);
            virtual void visit(NDQOpGen::Ref ref);
            virtual void visit(NDBinOp::Ref ref);
            virtual void visit(NDUnaryOp::Ref ref);
            virtual void visit(NDIdRef::Ref ref);
            virtual void visit(NDList::Ref ref);
            virtual void visit(NDStmtList::Ref ref);
            virtual void visit(NDGOpList::Ref ref);
            virtual void visit(NDIfStmt::Ref ref);
            virtual void visit(NDValue<std::string>::Ref ref);
            virtual void visit(NDValue<IntVal>::Ref ref);
            virtual void visit(NDValue<RealVal>::Ref ref);

            /// \brief Visits the children of \p ref.
            void visitChildren(Node::Ref ref);
    };

};

#endif
