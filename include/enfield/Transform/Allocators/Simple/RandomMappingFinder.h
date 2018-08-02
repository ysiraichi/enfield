#ifndef __EFD_RANDOM_MAPPING_FINDER_H__
#define __EFD_RANDOM_MAPPING_FINDER_H__

#include "enfield/Transform/Allocators/SimpleQAllocator.h"
#include "enfield/Support/WeightedGraph.h"

namespace efd {
    /// \brief Randomizes the mapping.
    class RandomMappingFinder : public MappingFinder {
        public:
            typedef RandomMappingFinder* Ref;
            typedef std::unique_ptr<RandomMappingFinder> uRef;

            Mapping find(ArchGraph::Ref g, DepsVector& deps) override;

            /// \brief Creates an instance of this class.
            static uRef Create();
    };
}

#endif
