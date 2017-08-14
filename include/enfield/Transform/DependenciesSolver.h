#ifndef __EFD_DEPENDENCIES_SOLVER_H__
#define __EFD_DEPENDENCIES_SOLVER_H__

#include "enfield/Transform/QbitAllocator.h"

namespace efd {
    /// \brief Interface for solving the dependencies (inserting swaps and other gates).
    class DependenciesSolver {
        public:
            typedef QbitAllocator::Mapping Mapping;
            typedef QbitAllocator::DepsSet DepsSet;

            typedef DependenciesSolver* Ref;
            typedef std::shared_ptr<DependenciesSolver> sRef;

            /// \brief Inserts swaps and other gates in order to solve the dependencies
            /// \p deps from the mapping \p initial, using the functions of \p allocator.
            virtual void solve(Mapping initial, DepsSet& deps, ArchGraph::Ref g,
                    QbitAllocator::Ref allocator) = 0;
    };
}

#endif
