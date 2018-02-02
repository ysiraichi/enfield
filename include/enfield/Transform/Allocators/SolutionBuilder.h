#ifndef __EFD_SOLUTION_BUILDER_H__
#define __EFD_SOLUTION_BUILDER_H__

#include "enfield/Transform/Allocators/QbitAllocator.h"
#include "enfield/Support/BitOptions.h"

namespace efd {
    /// \brief Options for solution builders.
    enum class SolutionBuilderOptions {
        ImproveInitial = 0,
        KeepStats
    };

    /// \brief Interface for building the solution from an initial mapping.
    class SolutionBuilder : public BitOptions<SolutionBuilderOptions,
                                              SolutionBuilderOptions::KeepStats> {
        public:
            typedef QbitAllocator::Mapping Mapping;
            typedef QbitAllocator::DepsSet DepsSet;

            typedef SolutionBuilder* Ref;
            typedef std::shared_ptr<SolutionBuilder> sRef;

        public:
            SolutionBuilder() {
                set(SolutionBuilderOptions::ImproveInitial);
                set(SolutionBuilderOptions::KeepStats);
            }

            /// \brief Constructs a solution (\em QbitAllocator::Solution) from the
            /// mapping \p initial, with \p deps dependencies in the architecture \p g.
            virtual Solution build(Mapping initial, DepsSet& deps, ArchGraph::Ref g) = 0;
    };
}

#endif
