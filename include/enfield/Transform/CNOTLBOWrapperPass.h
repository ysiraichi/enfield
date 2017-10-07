#ifndef __EFD_CNOT_LBO_WRAPPER_PASS_H__
#define __EFD_CNOT_LBO_WRAPPER_PASS_H__

#include "enfield/Transform/LayerBasedOrderingWrapperPass.h"

namespace efd {
    /// \brief Layer-based Ordering with CNOT priority.
    class CNOTLBOWrapperPass : public LayerBasedOrderingWrapperPass {
        public:
            typedef std::unique_ptr<CNOTLBOWrapperPass> uRef;
            typedef CNOTLBOWrapperPass* Ref;

            static uint8_t ID;

        protected:
            Ordering generate(CircuitGraph& graph) override;

        public:
            static uRef Create();
    };
}

#endif
