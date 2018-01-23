#ifndef __EFD_SEMANTIC_VERIFIER_H__
#define __EFD_SEMANTIC_VERIFIER_H__

#include "enfield/Transform/Pass.h"
#include "enfield/Transform/QModule.h"
#include "enfield/Support/Defs.h"

namespace efd {
    class SemanticVerifierPass : public PassT<bool> {
        public:
            typedef SemanticVerifierPass* Ref;
            typedef std::unique_ptr<SemanticVerifierPass> uRef;

        private:
            QModule::uRef mSrc;
            Mapping mInitial;

        public:
            /// \brief Constructs a verifier from a clone of the original semantics
            /// (the \p src QModule).
            SemanticVerifierPass(QModule::uRef src, Mapping initial);

            bool run(QModule* dst) override;

            /// \brief Create a dynamic instance of this class.
            static uRef Create(QModule::uRef src, Mapping initial);
    };
}

#endif
