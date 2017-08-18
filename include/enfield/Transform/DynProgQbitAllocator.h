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

        private:
            DynProgQbitAllocator(ArchGraph::sRef archGraph);

            MapResult dynsolve(std::vector<Dependencies>& deps);
            unsigned getIntermediateV(unsigned u, unsigned v);

        public:
            Mapping solveDependencies(DepsSet& deps) override;

            /// \brief Create a new instance of this class.
            static uRef Create(ArchGraph::sRef archGraph);
    };
}

#endif
