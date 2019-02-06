#ifndef __EFD_CHALLENGE_WINNER_QALLOCATOR_H__
#define __EFD_CHALLENGE_WINNER_QALLOCATOR_H__

#include "enfield/Transform/Allocators/QbitAllocator.h"
#include "enfield/Support/BFSCachedDistance.h"

namespace efd {

    /// \brief The IBM Challenge winner algorithm.
    ///
    /// Implemented from Zulehner et. al.:
    /// Compiling SU(4) Quantum Circuits to IBM QX Architectures.
    /// 
    /// Main python implementation can be found at:
    /// http://iic.jku.at/eda/research/ibm_qx_mapping/
    class ChallengeWinnerQAllocator : public QbitAllocator {
        public:
            typedef ChallengeWinnerQAllocator* Ref;
            typedef std::unique_ptr<ChallengeWinnerQAllocator> uRef;

        private:
            using UIntPair = std::pair<uint32_t, uint32_t>;

            BFSCachedDistance mDistance;

            uint32_t getOrAssignPQubitFor(uint32_t a,
                                          const Mapping& mapping,
                                          const InverseMap& inverse);

            SwapSeq astar(const std::vector<Dep>& dependencies,
                          const Mapping& mapping,
                          const InverseMap& inverse,
                          const std::set<UIntPair>& freeSwaps);

        protected:
            ChallengeWinnerQAllocator(ArchGraph::sRef ag);
            Mapping allocate(QModule::Ref qmod) override;

        public:
            static uRef Create(ArchGraph::sRef ag);
    };
}

#endif
