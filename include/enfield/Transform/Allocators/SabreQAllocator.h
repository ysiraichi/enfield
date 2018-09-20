#ifndef __EFD_SABRE_QALLOCATOR_H__
#define __EFD_SABRE_QALLOCATOR_H__

#include "enfield/Transform/Allocators/QbitAllocator.h"
#include "enfield/Transform/DependencyBuilderPass.h"
#include "enfield/Support/BFSCachedDistance.h"

#include <random>
#include <queue>

namespace efd {
    /// \brief SABRE QAllocator
    ///
    /// Implemented from Gushu et. al.:
    /// Tackling the Qubit Mapping Problem for NISQ-Era Quantum Devices
    class SabreQAllocator : public QbitAllocator {
        public:
            typedef SabreQAllocator* Ref;
            typedef std::unique_ptr<SabreQAllocator> uRef;

        private:
            typedef std::pair<Mapping, uint32_t> MappingAndNSwaps;

            uint32_t mLookAhead;
            uint32_t mIterations;
            BFSCachedDistance mBFSDistance;
            XbitToNumber mXbitToNumber;

            MappingAndNSwaps allocateWithInitialMapping(const Mapping& initialMapping,
                                                        QModule::Ref qmod,
                                                        bool issueInstructions);

        protected:
            SabreQAllocator(ArchGraph::sRef ag);
            Mapping allocate(QModule::Ref qmod) override;

        public:
            static uRef Create(ArchGraph::sRef ag);
    };
}

#endif
