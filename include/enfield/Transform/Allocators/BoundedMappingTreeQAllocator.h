#ifndef __EFD_BOUNDED_MAPPING_TREE_QALLOCATOR_H__
#define __EFD_BOUNDED_MAPPING_TREE_QALLOCATOR_H__

#include "enfield/Transform/Allocators/QbitAllocator.h"

namespace efd {
    struct AvailableCandidateGetter {
        typedef AvailableCandidateGetter* Ref;
        typedef std::unique_ptr<AvailableCandidateGetter> uRef;
        virtual std::vector<Node::Ref> get() = 0;
    };

    class BoundedMappingTreeQAllocator : public QbitAllocator {
        public:
            typedef BoundedMappingTreeQAllocator* Ref;
            typedef std::unique_ptr<BoundedMappingTreeQAllocator> uRef;

        protected:
            struct Candidate {
                Mapping m;
                uint32_t cost;
            };

            typedef std::vector<Candidate> CandidateVector;
            typedef std::vector<uint32_t> Vector;
            typedef std::vector<Vector> Matrix;

            uint32_t mPQubits;
            uint32_t mVQubits;
            Matrix mDistance;
            mArchGraph::sRef mArch;

            AvailableCandidateGetter::uRef ACGetter;

            Vector distanceFrom(uint32_t u);
            void preCalculateDistance();

            void phase1();

            BoundedMappingTreeQAllocator(ArchGraph::sRef ag);
            Solution executeAllocation(QModule::Ref qmod) override;

        public:
            static uRef Create(ArchGraph::sRef ag);
    };
}

#endif
