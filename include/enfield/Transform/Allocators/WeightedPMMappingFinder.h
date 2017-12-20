#ifndef __EFD_WEIGHTED_PM_MAPPING_FINDER_H__
#define __EFD_WEIGHTED_PM_MAPPING_FINDER_H__

#include "enfield/Transform/Allocators/MappingFinder.h"
#include "enfield/Support/PartialMatchingFinder.h"
#include "enfield/Support/WeightedGraph.h"

namespace efd {
    /// \brief Finds a mapping with a weighted partial matching algorithm (subgraph
    /// isomorphism).
    class WeightedPMMappingFinder : public MappingFinder {
        public:
            typedef uint32_t WeightTy;

            typedef WeightedPMMappingFinder* Ref;
            typedef std::unique_ptr<WeightedPMMappingFinder> uRef;

        private:
            PartialMatchingFinder::sRef mPMFinder;

        public:
            Mapping find(ArchGraph::Ref g, DepsSet& deps) override;

            /// \brief Sets the partial matching finder to \p finder.
            void setPMFinder(PartialMatchingFinder::sRef finder);

            /// \brief Creates an instance of this class.
            static uRef Create();
    };
}

#endif
