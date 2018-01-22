#ifndef __EFD_SEMANTIC_VERIFIER_H__
#define __EFD_SEMANTIC_VERIFIER_H__

#include "enfield/Transform/Pass.h"
#include "enfield/Transform/QModule.h"

namespace efd {
    class SemanticVerifierPass : public PassT<bool> {
        private:
            bool mSuccess;
            QModule::uRef mSrc;

        public:
            /// \brief Constructs a verifier from a clone of the original semantics
            /// (the \p src QModule).
            SemanticVerifierPass(QModule::uRef src);

            bool run(QModule* dst) override;
    };
}

#endif
