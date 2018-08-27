#ifndef __EFD_ERROR_RATE_CALCULATION_PASS_H__
#define __EFD_ERROR_RATE_CALCULATION_PASS_H__

#include "enfield/Transform/QModule.h"
#include "enfield/Transform/Pass.h"
#include "enfield/Arch/ArchGraph.h"

#include <map>

namespace efd {
    /// \brief Calculates the approximate error rate of a program.
    class ErrorRateCalculationPass : public PassT<double> {
        public:
            typedef ErrorRateCalculationPass* Ref;
            typedef std::unique_ptr<ErrorRateCalculationPass> uRef;
            typedef std::shared_ptr<ErrorRateCalculationPass> sRef;

            static uint8_t ID;

        private:
            ArchGraph::sRef mArchGraph;

        public:
            ErrorRateCalculationPass(ArchGraph::sRef arch = nullptr);

            /// \brief Sets the \em ArchGraph that this pass should use for calculating
            /// the error rate.
            void setArchGraph(ArchGraph::sRef arch);

            bool run(QModule::Ref qmod) override;

            /// \brief Returns a new instance of this class.
            static uRef Create(ArchGraph::sRef arch = nullptr);
    };
};

#endif
