#ifndef __EFD_SIMPLE_QBIT_ALLOCATOR_H__
#define __EFD_SIMPLE_QBIT_ALLOCATOR_H__

#include "enfield/Transform/QbitAllocator.h"
#include "enfield/Transform/MappingFinder.h"
#include "enfield/Transform/DependenciesSolver.h"

namespace efd {
    /// \brief QbitAllocator that allocates the logical qubits from an initial mapping.
    class SimpleQbitAllocator : public QbitAllocator {
        public:
            typedef SimpleQbitAllocator* Ref;
            typedef std::unique_ptr<SimpleQbitAllocator> uRef;

        protected:
            MappingFinder::sRef mMapFinder;
            DependenciesSolver::sRef mDepSolver;

            SimpleQbitAllocator(ArchGraph::sRef agraph);

        public:
            Mapping solveDependencies(DepsSet& deps) override;

            /// \brief Sets the mapping finder to \p finder.
            void setMapFinder(MappingFinder::sRef finder);
            /// \brief Sets the dependency solver to \p solver.
            void setDepSolver(DependenciesSolver::sRef solver);

            /// \brief Creates an instance of this class.
            static uRef Create(ArchGraph::sRef agraph);
    };
}

#endif
