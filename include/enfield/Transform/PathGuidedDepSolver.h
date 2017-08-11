#ifndef __EFD_PATH_GUIDED_DEP_SOLVER_H__
#define __EFD_PATH_GUIDED_DEP_SOLVER_H__

#include "enfield/Transform/DependenciesSolver.h"
#include "enfield/Support/PathFinder.h"

namespace efd {
    /// \brief Solves the dependencies by looking at the path in the graph.
    class PathGuidedDepSolver : public DependenciesSolver {
        public:
            typedef PathGuidedDepSolver* Ref;
            typedef std::unique_ptr<PathGuidedDepSolver> uRef;

        protected:
            PathFinder::sRef mPathFinder;

        public:
            void solve(Mapping initial, DepsSet& deps, ArchGraph::sRef agraph,
                    QbitAllocator::Ref allocator) override;

            /// \brief Sets the path finder to be used.
            void setPathFinder(PathFinder::sRef finder);

            /// \brief Creates an instance of this class.
            static uRef Create();
    };
}

#endif
