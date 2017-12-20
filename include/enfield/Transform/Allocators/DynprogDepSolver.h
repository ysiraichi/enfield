#ifndef __EFD_DYNPROG_DEP_SOLVER_H__
#define __EFD_DYNPROG_DEP_SOLVER_H__

#include "enfield/Transform/Allocators/DepSolverQAllocator.h"

#include <unordered_map>
#include <string>

struct MapResult;

namespace efd {
    /// \brief Implementation of DepSolverQAllocator that uses dynamic programming to
    /// obtain an optimal solution.
    class DynprogDepSolver : public DepSolverQAllocator {
        public:
            typedef DynprogDepSolver* Ref;
            typedef std::unique_ptr<DynprogDepSolver> uRef;

        protected:
            DynprogDepSolver(ArchGraph::sRef archGraph);

            /// \brief Gets the intermediate vertex between 'u' and 'v', if
            /// there exists one.
            uint32_t getIntermediateV(uint32_t u, uint32_t v);

            Solution solve(DepsSet& deps) override;

        public:
            /// \brief Create a new instance of this class.
            static uRef Create(ArchGraph::sRef archGraph);
    };
}

#endif
