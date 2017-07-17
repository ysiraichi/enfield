#ifndef __EFD_SWAP_FINDER_H__
#define __EFD_SWAP_FINDER_H__

#include "enfield/Support/Graph.h"

namespace efd {
    /// \brief Interface for finding the swaps to be done, given a number of
    /// restrictions.
    class SwapFinder {
        public:
            typedef SwapFinder* Ref;
            typedef std::shared_ptr<SwapFinder> sRef;

        protected:
            Graph::sRef mG;

            SwapFinder(Graph::sRef g) : mG(g) {}

        public:
            /// \brief Struct for representing swaps.
            struct Swap {
                unsigned mU;
                unsigned mV;
            };

            struct Rest {
                unsigned mFrom;
                unsigned mTo;
            };

            typedef std::vector<Rest> RestrictionVector;
            typedef std::vector<Swap> SwapVector;

            /// \brief Given a std::vector of restrictions (edges in a graph), it returns
            /// a std::vector of swaps necessary to be done.
            virtual SwapVector findSwaps(RestrictionVector restrictions) = 0;
    };
}

#endif
