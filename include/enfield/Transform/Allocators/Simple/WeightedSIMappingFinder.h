#ifndef __EFD_WEIGHTED_SI_MAPPING_FINDER_H__
#define __EFD_WEIGHTED_SI_MAPPING_FINDER_H__

#include "enfield/Transform/Allocators/SimpleQAllocator.h"
#include "enfield/Support/SIFinder.h"
#include "enfield/Support/WeightedGraph.h"

namespace efd {
    /// \brief Finds a mapping with a weighted partial matching algorithm (subgraph
    /// isomorphism).
    class WeightedSIMappingFinder : public MappingFinder {
        public:
            typedef uint32_t WeightTy;

            typedef WeightedSIMappingFinder* Ref;
            typedef std::unique_ptr<WeightedSIMappingFinder> uRef;

        private:
            SIFinder::sRef mSIFinder;

        public:
            Mapping find(ArchGraph::Ref g, DepsVector& deps) override;

            /// \brief Sets the subgraph isomorphism finder to \p finder.
            void setSIFinder(SIFinder::sRef finder);

            /// \brief Creates an instance of this class.
            static uRef Create();
    };
}

#endif
