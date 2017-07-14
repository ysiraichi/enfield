#ifndef __EFD_QMODULEFY_PASS_H__
#define __EFD_QMODULEFY_PASS_H__

#include "enfield/Pass.h"

#include <set>

namespace efd {
    class IdTable;
    class QModule;

    class QModulefyPass : public Pass {
        public:
            typedef QModulefyPass* Ref;
            typedef std::unique_ptr<QModulefyPass> uRef;
            typedef std::shared_ptr<QModulefyPass> sRef;

        private:
            std::set<std::string> mIncludes;
            QModulefyPass(std::shared_ptr<QModule> qmod);

        protected:
            void initImpl(bool force) override;

        public:
            std::shared_ptr<QModule> mMod;
            IdTable* mCurrentTable;

            void visit(NDQasmVersion::Ref ref) override;
            void visit(NDInclude::Ref ref) override;
            void visit(NDDecl::Ref ref) override;
            void visit(NDGateDecl::Ref ref) override;
            void visit(NDOpaque::Ref ref) override;
            void visit(NDQOpMeasure::Ref ref) override;
            void visit(NDQOpReset::Ref ref) override;
            void visit(NDQOpU::Ref ref) override;
            void visit(NDQOpCX::Ref ref) override;
            void visit(NDQOpBarrier::Ref ref) override;
            void visit(NDQOpGeneric::Ref ref) override;
            void visit(NDStmtList::Ref ref) override;
            void visit(NDIfStmt::Ref ref) override;

            static uRef Create(std::shared_ptr<QModule> qmod);
    };
};

#endif
