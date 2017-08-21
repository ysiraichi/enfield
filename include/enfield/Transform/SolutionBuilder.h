#ifndef __EFD_SOLUTION_BUILDER_H__
#define __EFD_SOLUTION_BUILDER_H__

#include "enfield/Transform/QbitAllocator.h"

namespace efd {
    /// \brief Interface for building the solution from an initial mapping.
    class SolutionBuilder {
        public:
            typedef QbitAllocator::Mapping Mapping;
            typedef QbitAllocator::Solution Solution;
            typedef QbitAllocator::DepsSet DepsSet;

            typedef SolutionBuilder* Ref;
            typedef std::shared_ptr<SolutionBuilder> sRef;

            /// \brief Constructs a solution (\em QbitAllocator::Solution) from the
            /// mapping \p initial, with \p deps dependencies in the architecture \p g.
            virtual Solution build(Mapping initial, DepsSet& deps, ArchGraph::Ref g) = 0;
    };
}

#endif
