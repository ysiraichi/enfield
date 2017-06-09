#ifndef __EFD_INLINE_ALL_PASS_H__
#define __EFD_INLINE_ALL_PASS_H__

#include "enfield/Pass.h"
#include "enfield/Transform/QModule.h"

#include <set>

namespace efd {
    class InlineAllPass : public Pass {
        private:
            QModule* mMod;
            std::set<std::string> mBasis;
            bool mInlined;
            
            void initImpl(bool force) override;
            void recursiveInline(NDQOpGeneric* ref);

        public:
            InlineAllPass(QModule* qmod, std::vector<std::string> basis =
                    std::vector<std::string>());

            void visit(NDQOpGeneric* ref) override;
            void visit(NDIfStmt* ref) override;

            /// \brief Returns whether this pass has inlined any gate.
            bool hasInlined() const;

            bool doesInvalidatesModule() const override;

            /// \brief Creates an instance of this pass.
            static InlineAllPass* Create(QModule* ref, std::vector<std::string> basis = 
                    std::vector<std::string>());
    };
}

#endif
