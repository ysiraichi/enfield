#ifndef __EFD_SEMANTIC_VERIFIER_PASS_H__
#define __EFD_SEMANTIC_VERIFIER_PASS_H__

#include "enfield/Transform/Pass.h"
#include "enfield/Transform/QModule.h"
#include "enfield/Support/Result.h"
#include "enfield/Support/Defs.h"

namespace efd {
    /// \brief Verifies the CNOT relations between every qubit.
    ///
    /// It should check whether the semantics were changed by any of the transformations
    /// applied to the given \em QModule.
    /// Note that it only knows how to verify CNOT gates (for gates that use more than one
    /// qubit). If it is not the case for the given module, one should either \em flatten it
    /// or transform the gate into single qubit gates.
    class SemanticVerifierPass : public PassT<ResultMsg> {
        public:
            typedef SemanticVerifierPass* Ref;
            typedef std::unique_ptr<SemanticVerifierPass> uRef;

        private:
            QModule::uRef mSrc;
            Mapping mInitial;
            std::vector<std::string> mBasis;

        public:
            /// \brief Constructs a verifier from a clone of the original semantics
            /// (the \p src QModule).
            SemanticVerifierPass(QModule::uRef src, Mapping initial);

            /// \brief Flags the verifier to inline all gates, but those inside the
            /// \p basis vector, before mapping.
            void setInlineAll(std::vector<std::string> basis = {});

            bool run(QModule* dst) override;

            /// \brief Create a dynamic instance of this class.
            static uRef Create(QModule::uRef src, Mapping initial);
    };
}

#endif
