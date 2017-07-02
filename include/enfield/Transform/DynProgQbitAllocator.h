#ifndef __EFD_DYN_PROG_QBIT_ALLOCATOR_H__
#define __EFD_DYN_PROG_QBIT_ALLOCATOR_H__

#include "enfield/Transform/QbitAllocator.h"

#include <unordered_map>
#include <string>

namespace efd {
    /// \brief Implementation of QbitAllocator that uses dynamic programming to
    /// obtain an optimal solution.
    class DynProgQbitAllocator : public QbitAllocator {
        private:
            DynProgQbitAllocator(QModule* qmod, ArchGraph* archGraph);

        public:
            Mapping solveDependencies(DepsSet& deps) override;

            /// \brief Create a new instance of this class.
            static DynProgQbitAllocator* Create(QModule* qmod, ArchGraph* archGraph);
    };
}

#endif
