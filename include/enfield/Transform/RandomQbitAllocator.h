#ifndef __EFD_RANDOM_QBIT_ALLOCATOR_H__
#define __EFD_RANDOM_QBIT_ALLOCATOR_H__

#include "enfield/Transform/QbitAllocator.h"

namespace efd {
    /// \brief QbitAllocator that allocates the qubits randomly.
    class RandomQbitAllocator : public QbitAllocator {
        public:
            typedef RandomQbitAllocator* Ref;
            typedef std::unique_ptr<RandomQbitAllocator> uRef;

        private:
            RandomQbitAllocator(ArchGraph::sRef agraph);

        public:
            Mapping solveDependencies(DepsSet& deps) override;

            /// \brief Creates an instance of this class.
            static uRef Create(ArchGraph::sRef agraph);
    };
}

#endif
