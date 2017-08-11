#ifndef __EFD_QBITTER_DEP_SOLVER_H__
#define __EFD_QBITTER_DEP_SOLVER_H__

#include "enfield/Transform/DependenciesSolver.h"

namespace efd {
    /// \brief Solves the dependencies using the 'long-cnot' gates.
    ///
    /// This algorithm was based on the implementation on the link:
    /// https://github.com/artiste-qb-net/qubiter/blob/master/Qubiter_to_IBMqasm.py 
    class QbitterDepSolver : public DependenciesSolver {
        public:
            typedef QbitterDepSolver* Ref;
            typedef std::unique_ptr<QbitterDepSolver> uRef;

            void solve(Mapping initial, DepsSet& deps, ArchGraph::sRef agraph,
                    QbitAllocator::Ref allocator) override;

            /// \brief Creates an instance of this class.
            static uRef Create();
    };
}

#endif
