#ifndef __EFD_SI_FINDER_H__
#define __EFD_SI_FINDER_H__

#include "enfield/Support/Graph.h"
#include "enfield/Support/Defs.h"

#include <vector>
#include <memory>

namespace efd {
    /// \brief Interface for implementing different algorithms for finding
    /// matchings (subgraph isomorphism).
    class SIFinder {
        public:
            typedef SIFinder* Ref;
            typedef std::unique_ptr<SIFinder> uRef;
            typedef std::shared_ptr<SIFinder> sRef;

            struct Result {
                bool success;
                Mapping m;
            };

            virtual ~SIFinder() = default;

            /// \brief Returns a valid matching of \p h in \p g.
            ///
            /// Note that this is not necessairly an exact match (exact 
            /// subgraph isomorphism). That is because this is a NP-Complete
            /// problem.
            virtual Result find(Graph::Ref g, Graph::Ref h) = 0;
    };
}

#endif
