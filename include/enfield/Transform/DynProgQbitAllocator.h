#ifndef __EFD_DYN_PROG_QBIT_ALLOCATOR_H__
#define __EFD_DYN_PROG_QBIT_ALLOCATOR_H__

#include "enfield/Transform/QbitAllocator.h"

#include <unordered_map>
#include <string>

namespace efd {
    /// \brief Implementation of QbitAllocator that uses dynamic programming to
    /// obtain an optimal solution.
    class DynProgQbitAllocator : public QbitAllocator {
        public:
            typedef DynProgQbitAllocator* Ref;
            typedef std::unique_ptr<DynProgQbitAllocator> uRef;

        private:
            DynProgQbitAllocator(QModule::sRef qmod, ArchGraph::sRef archGraph);

        public:
            Mapping solveDependencies(DepsSet& deps) override;

            /// \brief Create a new instance of this class.
            static uRef Create(QModule::sRef qmod, ArchGraph::sRef archGraph);
    };
}

#endif
