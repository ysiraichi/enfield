#ifndef __EFD_WEIGHTED_PM_QBIT_ALLOCATOR_H__
#define __EFD_WEIGHTED_PM_QBIT_ALLOCATOR_H__

#include "enfield/Transform/QbitAllocator.h"
#include "enfield/Support/WeightedGraph.h"
#include "enfield/Support/WeightedPMFinder.h"
#include "enfield/Support/PathFinder.h"

namespace efd {
    /// \brief QbitAllocator that uses a partial graph pattern matching
    /// (subgraph isomorphism).
    class WeightedPMQbitAllocator : public QbitAllocator {
        public:
            typedef unsigned WeightTy;
            typedef WeightedPMQbitAllocator* Ref;
            typedef std::unique_ptr<WeightedPMQbitAllocator> uRef;

        private:
            WeightedGraph<WeightTy>::uRef mWG;
            WeightedPMFinder<WeightTy>::sRef mPMFinder;
            PathFinder::sRef mPathFinder;

            WeightedPMQbitAllocator(ArchGraph::sRef agraph);

            std::unique_ptr<WeightedGraph<WeightTy>> createWG(DepsSet& deps);
            std::vector<unsigned> genAssign(std::vector<unsigned> match);

        public:
            Mapping solveDependencies(DepsSet& deps) override;

            void setWPMFinder(WeightedPMFinder<WeightTy>::sRef finder);
            void setPathFinder(PathFinder::sRef finder);

            /// \brief Creates an instance of this class.
            static uRef Create(ArchGraph::sRef agraph);
    };
}

#endif
