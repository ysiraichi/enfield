#ifndef __EFD_DEP_SOLVER_QALLOCATOR_H__
#define __EFD_DEP_SOLVER_QALLOCATOR_H__

#include "enfield/Transform/Allocators/QbitAllocator.h"

namespace efd {
    class DepSolverQAllocator : public QbitAllocator {
        protected:
            DepSolverQAllocator(ArchGraph::sRef archGraph);

            /// \brief Solves the allocation problem, returning a solution.
            virtual Solution solve(DepsSet& deps) = 0;

            /// \brief Executes the allocation algorithm after the preprocessing.
            virtual Solution executeAllocation(QModule::Ref qmod) override;
    };
}

#endif
