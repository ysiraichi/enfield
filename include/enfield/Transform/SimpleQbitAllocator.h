#ifndef __EFD_SIMPLE_QBIT_ALLOCATOR_H__
#define __EFD_SIMPLE_QBIT_ALLOCATOR_H__

#include "enfield/Transform/QbitAllocator.h"
#include "enfield/Transform/MappingFinder.h"
#include "enfield/Transform/SolutionBuilder.h"

namespace efd {
    /// \brief QbitAllocator that allocates the logical qubits from an initial mapping.
    class SimpleQbitAllocator : public QbitAllocator {
        public:
            typedef SimpleQbitAllocator* Ref;
            typedef std::unique_ptr<SimpleQbitAllocator> uRef;

        protected:
            MappingFinder::sRef mMapFinder;
            SolutionBuilder::sRef mSolBuilder;

            SimpleQbitAllocator(ArchGraph::sRef agraph);

            Solution solve(DepsSet& deps) override;

        public:
            /// \brief Sets the mapping finder to \p finder.
            void setMapFinder(MappingFinder::sRef finder);
            /// \brief Sets the solution builder to \p builder.
            void setSolBuilder(SolutionBuilder::sRef builder);

            /// \brief Creates an instance of this class.
            static uRef Create(ArchGraph::sRef agraph);
    };
}

#endif
