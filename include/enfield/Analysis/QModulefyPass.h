#ifndef __EFD_QMODULEFY_PASS_H__
#define __EFD_QMODULEFY_PASS_H__

#include "enfield/Analysis/NodeVisitor.h"
#include "enfield/Analysis/QModule.h"

namespace efd {
    class IdTable;

    class QModulefyPass : public NodeVisitor {
        private:
            QModulefyPass();

        public:
            QModule* mMod;
            IdTable* mCurrentTable;

            void visit(NDQasmVersion* ref) override;
            void visit(NDInclude* ref) override;
            void visit(NDDecl* ref) override;
            void visit(NDGateDecl* ref) override;
            void visit(NDOpaque* ref) override;
            void visit(NDQOpMeasure* ref) override;
            void visit(NDQOpReset* ref) override;
            void visit(NDQOpU* ref) override;
            void visit(NDQOpCX* ref) override;
            void visit(NDQOpBarrier* ref) override;
            void visit(NDQOpGeneric* ref) override;
            void visit(NDStmtList* ref) override;
            void visit(NDIfStmt* ref) override;

            static QModulefyPass* Create();
    };
};

#endif
