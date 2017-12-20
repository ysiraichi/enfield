#ifndef __EFD_SIMPLE_QBIT_ALLOCATOR_H__
#define __EFD_SIMPLE_QBIT_ALLOCATOR_H__

#include "enfield/Transform/Allocators/DepSolverQAllocator.h"
#include "enfield/Transform/Allocators/MappingFinder.h"
#include "enfield/Transform/Allocators/SolutionBuilder.h"

namespace efd {
    /// \brief DependencySolver that allocates the logical qubits from an initial mapping.
    class SimpleDepSolver : public DepSolverQAllocator {
        public:
            typedef SimpleDepSolver* Ref;
            typedef std::unique_ptr<SimpleDepSolver> uRef;

        protected:
            MappingFinder::sRef mMapFinder;
            SolutionBuilder::sRef mSolBuilder;

            SimpleDepSolver(ArchGraph::sRef agraph);

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
