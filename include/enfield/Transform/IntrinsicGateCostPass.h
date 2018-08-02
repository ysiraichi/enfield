#ifndef __EFD_INTRINSIC_GATE_COST_PASS_H__
#define __EFD_INTRINSIC_GATE_COST_PASS_H__

#include "enfield/Transform/QModule.h"
#include "enfield/Transform/Pass.h"

namespace efd {
    class IntrinsicGateCostPass : public PassT<uint32_t> {
        public:
            typedef IntrinsicGateCostPass* Ref;
            typedef std::unique_ptr<IntrinsicGateCostPass> uRef;
            typedef std::shared_ptr<IntrinsicGateCostPass> sRef;

            static uint8_t ID;

            bool run(QModule::Ref qmod) override;

            /// \brief Returns a new instance of this class.
            static uRef Create();
    };
};

#endif
