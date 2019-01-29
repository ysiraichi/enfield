#ifndef __EFD_CHALLENGE_WINNER_QALLOCATOR_H__
#define __EFD_CHALLENGE_WINNER_QALLOCATOR_H__

#include "enfield/Transform/Allocators/QbitAllocator.h"
#include "enfield/Support/BFSCachedDistance.h"

namespace efd {
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
