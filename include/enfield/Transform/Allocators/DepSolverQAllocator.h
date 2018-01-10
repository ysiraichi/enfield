#ifndef __EFD_DEP_SOLVER_QALLOCATOR_H__
#define __EFD_DEP_SOLVER_QALLOCATOR_H__

#include "enfield/Transform/Allocators/QbitAllocator.h"

namespace efd {
    /// \brief Wrapper for making it easier to implement algorithms for \em Qubit Allocation.
    ///
    /// It just gives a set of dependencies (note that it can be nested) and outputs a solution
    /// for the current instance.
    class DepSolverQAllocator : public QbitAllocator {
        protected:
            DepSolverQAllocator(ArchGraph::sRef archGraph);

            /// \brief Solves the allocation problem, returning a solution.
            virtual Solution solve(DepsSet& deps) = 0;
            virtual Solution executeAllocation(QModule::Ref qmod) override;
    };
}

#endif
