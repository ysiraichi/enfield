#ifndef __EFD_PATH_GUIDED_SOL_BUILDER_H__
#define __EFD_PATH_GUIDED_SOL_BUILDER_H__

#include "enfield/Transform/Allocators/SimpleQAllocator.h"
#include "enfield/Support/PathFinder.h"

namespace efd {
    /// \brief Solves the dependencies by looking at the path in the graph.
    class PathGuidedSolBuilder : public SolutionBuilder {
        public:
            typedef PathGuidedSolBuilder* Ref;
            typedef std::unique_ptr<PathGuidedSolBuilder> uRef;

        protected:
            PathFinder::sRef mPathFinder;

        public:
            StdSolution build(Mapping initial, DepsVector& deps, ArchGraph::Ref g) override;

            /// \brief Sets the path finder to be used.
            void setPathFinder(PathFinder::sRef finder);

            /// \brief Creates an instance of this class.
            static uRef Create();
    };
}

#endif
