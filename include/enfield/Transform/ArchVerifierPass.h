#ifndef __EFD_ARCH_VERIFIER_PASS_H__
#define __EFD_ARCH_VERIFIER_PASS_H__

#include "enfield/Transform/Pass.h"
#include "enfield/Transform/QModule.h"
#include "enfield/Arch/ArchGraph.h"
#include "enfield/Support/Defs.h"

namespace efd {
    /// \brief Verifies if the CNOT relations between every qubit respects the architecture
    /// constraints.
    class ArchVerifierPass : public PassT<bool> {
        public:
            typedef ArchVerifierPass* Ref;
            typedef std::unique_ptr<ArchVerifierPass> uRef;

        private:
            ArchGraph::sRef mArch;

        public:
            ArchVerifierPass(ArchGraph::sRef ag);

            bool run(QModule* qmod) override;

            /// \brief Create a dynamic instance of this class.
            static uRef Create(ArchGraph::sRef ag);
    };
}

#endif
