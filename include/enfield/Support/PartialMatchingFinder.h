#ifndef __EFD_PARTIAL_MATCHING_FINDER_H__
#define __EFD_PARTIAL_MATCHING_FINDER_H__

#include "enfield/Support/Graph.h"

#include <vector>
#include <memory>

namespace efd {
    /// \brief Interface for implementing different algorithms for finding
    /// partial matchings (approximate subgraph isomorphism).
    class PartialMatchingFinder {
        public:
            typedef PartialMatchingFinder* Ref;
            typedef std::unique_ptr<PartialMatchingFinder> uRef;
            typedef std::shared_ptr<PartialMatchingFinder> sRef;
            
        protected:
            Graph::sRef mG;

            PartialMatchingFinder(Graph::sRef g) : mG(g) {}

        public:
            /// \brief Returns a valid matching.
            /// Note that this is not necessairly an exact match (exact 
            /// subgraph isomorphism). That is because this is a NP-Complete
            /// problem.
            virtual std::vector<unsigned> find(Graph::Ref h) = 0;
    };
}

#endif
