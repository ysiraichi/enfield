#ifndef __EFD_MAPPING_FINDER_H__
#define __EFD_MAPPING_FINDER_H__

#include "enfield/Transform/QbitAllocator.h"

namespace efd {
    /// \brief Interface for finding a mapping from some set of dependencies.
    class MappingFinder {
        public:
            typedef QbitAllocator::Mapping Mapping;
            typedef QbitAllocator::DepsSet DepsSet;

            typedef MappingFinder* Ref;
            typedef std::shared_ptr<MappingFinder> sRef;

            /// \brief Returns a mapping generated from a set of dependencies.
            virtual Mapping find(ArchGraph::sRef agraph, DepsSet& deps) = 0;
    };
}

#endif
