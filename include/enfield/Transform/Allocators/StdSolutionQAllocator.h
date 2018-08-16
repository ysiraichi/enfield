#ifndef __EFD_STD_SOLUTION_QALLOCATOR_H__
#define __EFD_STD_SOLUTION_QALLOCATOR_H__

#include "enfield/Transform/Allocators/QbitAllocator.h"

namespace efd {
    /// \brief Struct used to describe the operation chosen for each solving each
    /// dependency.
    ///
    /// Probably we should construct an union for all operations. For the time being,
    /// only the lcnot uses more than one property.
    struct Operation {
        /// \brief Kinds of operations available.
        enum Kind {
            K_OP_CNOT,
            K_OP_REV,
            K_OP_LCNOT,
            K_OP_SWAP
        };

        Kind mK;
        uint32_t mU, mV;
        /// \brief The intermediate vertex for using the long cnot.
        uint32_t mW;
    };

    /// \brief The solution for the allocation problem.
    ///
    /// It consists in an initial mapping L:P->V (\em mInitial), a sequence of sequences
    /// of operations previously defined (\em mOpSeqs) and the final cost (\em mCost).
    struct StdSolution {
        typedef std::vector<Operation> OpVector;
        typedef std::vector<std::pair<Node::Ref, OpVector>> OpSequences;
        typedef std::vector<uint32_t> Mapping;

        Mapping mInitial;
        OpSequences mOpSeqs;
    };

    /// \brief An abstract allocator that builds the solution based on a `StdSolution`.
    ///
    /// It runs `buildStdSolution` for getting an `StdSolution`, and then processes it
    /// along with the `QModule` inserting and replacing instructions where needed.
    class StdSolutionQAllocator : public QbitAllocator {
        public:
            typedef StdSolutionQAllocator* Ref;
            typedef std::unique_ptr<StdSolutionQAllocator> uRef;

            StdSolutionQAllocator(ArchGraph::sRef archGraph);

        protected:
            /// \brief Executes the allocation algorithm after the preprocessing.
            virtual StdSolution buildStdSolution(QModule::Ref qmod) = 0;

            Mapping allocate(QModule::Ref qmod) override;
    };
}

#endif
