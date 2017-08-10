#ifndef __EFD_QBITTER_QBIT_ALLOCATOR_H__
#define __EFD_QBITTER_QBIT_ALLOCATOR_H__

#include "enfield/Transform/QbitAllocator.h"

#include <unordered_map>
#include <string>

namespace efd {
    /// \brief Implementation of QbitAllocator based on the Qbitter.
    ///
    /// Link:
    /// https://github.com/artiste-qb-net/qubiter/blob/master/Qubiter_to_IBMqasm.py 
    class QbitterQbitAllocator : public QbitAllocator {
        public:
            typedef QbitterQbitAllocator* Ref;
            typedef std::unique_ptr<QbitterQbitAllocator> uRef;

        private:
            QbitterQbitAllocator(ArchGraph::sRef archGraph);

        public:
            Mapping solveDependencies(DepsSet& deps) override;

            /// \brief Create a new instance of this class.
            static uRef Create(ArchGraph::sRef archGraph);
    };
}

#endif
