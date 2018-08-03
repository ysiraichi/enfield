#ifndef __EFD_QMODULE_QUALITY_EVAL_PASS_H__
#define __EFD_QMODULE_QUALITY_EVAL_PASS_H__

#include "enfield/Transform/QModule.h"
#include "enfield/Transform/Pass.h"

#include <map>

namespace efd {
    /// \brief Holds the evaluation metrics for an allocation.
    struct QModuleQuality {
        uint32_t mDepth;
        uint32_t mGates;
        uint32_t mWeightedCost;
    };

    /// \brief Evaluates a `QModule`.
    ///
    /// Computes the depth of the circuit, the number of gates, and
    /// a weighted sum for each gate.
    class QModuleQualityEvalPass : public PassT<QModuleQuality> {
        public:
            typedef QModuleQualityEvalPass* Ref;
            typedef std::unique_ptr<QModuleQualityEvalPass> uRef;
            typedef std::shared_ptr<QModuleQualityEvalPass> sRef;
            typedef std::map<std::string, uint32_t> MapGateUInt;

        private:
            MapGateUInt mGatesW;

        public:
            static uint8_t ID;

            QModuleQualityEvalPass(MapGateUInt gatesW);
            bool run(QModule::Ref qmod) override;

            /// \brief Returns a new instance of this class.
            static uRef Create(MapGateUInt gatesW);
    };
};

#endif
