#ifndef __EFD_DYN_PROG_QBIT_ALLOCATOR_H__
#define __EFD_DYN_PROG_QBIT_ALLOCATOR_H__

#include "enfield/Transform/QbitAllocator.h"

#include <unordered_map>
#include <string>

struct MapResult;

namespace efd {
    /// \brief Implementation of QbitAllocator that uses dynamic programming to
    /// obtain an optimal solution.
    class DynProgQbitAllocator : public QbitAllocator {
        public:
            typedef DynProgQbitAllocator* Ref;
            typedef std::unique_ptr<DynProgQbitAllocator> uRef;

        protected:
            DynProgQbitAllocator(ArchGraph::sRef archGraph);

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
