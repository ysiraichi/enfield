#ifndef __EFD_GREEDY_CKT_QALLOCATOR_H__
#define __EFD_GREEDY_CKT_QALLOCATOR_H__

#include "enfield/Transform/Allocators/QbitAllocator.h"

namespace efd {
    class GreedyCktQAllocator : public QbitAllocator {
        public:
            typedef GreedyCktQAllocator* Ref;
            typedef std::unique_ptr<GreedyCktQAllocator> uRef;

        protected:
            GreedyCktQAllocator(ArchGraph::sRef ag);
            Solution executeAllocation(QModule::Ref qmod) override;

        public:
            static uRef Create(ArchGraph::sRef ag);
    };
}

#endif
