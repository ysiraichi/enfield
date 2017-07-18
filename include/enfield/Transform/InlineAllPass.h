#ifndef __EFD_INLINE_ALL_PASS_H__
#define __EFD_INLINE_ALL_PASS_H__

#include "enfield/Pass.h"
#include "enfield/Transform/QModule.h"

#include <set>

namespace efd {
    class InlineAllPass : public Pass {
        public:
            typedef InlineAllPass* Ref;
            typedef std::unique_ptr<InlineAllPass> uRef;

        private:
            QModule::sRef mMod;
            std::set<std::string> mBasis;
            bool mInlined;
            
            void initImpl(bool force) override;
            void recursiveInline(NDQOpGeneric::Ref ref);

        public:
            InlineAllPass(QModule::sRef qmod, std::vector<std::string> basis =
                    std::vector<std::string>());

            void visit(NDQOpGeneric::Ref ref) override;
            void visit(NDIfStmt::Ref ref) override;

            /// \brief Returns whether this pass has inlined any gate.
            bool hasInlined() const;

            bool doesInvalidatesModule() const override;

            /// \brief Creates an instance of this pass.
            static uRef Create(QModule::sRef ref, std::vector<std::string> basis = 
                    std::vector<std::string>());
    };
}

#endif
