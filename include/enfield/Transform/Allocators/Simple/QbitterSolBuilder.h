#ifndef __EFD_QBITTER_SOL_BUILDER_H__
#define __EFD_QBITTER_SOL_BUILDER_H__

#include "enfield/Transform/Allocators/SimpleQAllocator.h"

namespace efd {
    /// \brief Solves the dependencies using the 'long-cnot' gates.
    ///
    /// This algorithm was based on the implementation on the link:
    /// https://github.com/artiste-qb-net/qubiter/blob/master/Qubiter_to_IBMqasm.py 
    class QbitterSolBuilder : public SolutionBuilder {
        public:
            typedef QbitterSolBuilder* Ref;
            typedef std::unique_ptr<QbitterSolBuilder> uRef;

            StdSolution build(Mapping initial, DepsVector& deps, ArchGraph::Ref g) override;

            /// \brief Creates an instance of this class.
            static uRef Create();
    };
}

#endif
