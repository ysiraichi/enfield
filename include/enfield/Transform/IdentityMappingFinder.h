#ifndef __EFD_IDENTITY_MAPPING_FINDER_H__
#define __EFD_IDENTITY_MAPPING_FINDER_H__

#include "enfield/Transform/MappingFinder.h"

namespace efd {
    /// \brief Finds a mapping where the logical qubit 0 is mapped to
    /// the physical qubit 0, etc.
    class IdentityMappingFinder : public MappingFinder {
        public:
            typedef IdentityMappingFinder* Ref;
            typedef std::unique_ptr<IdentityMappingFinder> uRef;

            Mapping find(ArchGraph::sRef agraph, DepsSet& deps) override;

            /// \brief Creates an instance of this class.
            static uRef Create();
    };
}

#endif
