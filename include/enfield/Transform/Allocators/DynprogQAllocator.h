#ifndef __EFD_DYNPROG_QALLOCATOR_H__
#define __EFD_DYNPROG_QALLOCATOR_H__

#include "enfield/Transform/Allocators/StdSolutionQAllocator.h"

#include <unordered_map>
#include <string>

struct MapResult;

namespace efd {
    /// \brief Implementation of DepSolverQAllocator that uses dynamic programming to
    /// obtain an optimal solution.
    class DynprogQAllocator : public StdSolutionQAllocator {
        public:
            typedef DynprogQAllocator* Ref;
            typedef std::unique_ptr<DynprogQAllocator> uRef;

        protected:
            DynprogQAllocator(ArchGraph::sRef archGraph);

            /// \brief Gets the intermediate vertex between 'u' and 'v', if
            /// there exists one.
            uint32_t getIntermediateV(uint32_t u, uint32_t v);

            StdSolution buildStdSolution(QModule::Ref qmod) override;

        public:
            /// \brief Create a new instance of this class.
            static uRef Create(ArchGraph::sRef archGraph);
    };
}

#endif
