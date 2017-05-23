#ifndef __EFD_SWAP_FINDING_H__
#define __EFD_SWAP_FINDING_H__

#include "enfield/Support/Graph.h"

namespace efd {
    /// \brief Interface for finding the swaps to be done, given a number of
    /// restrictions.
    class SwapFinding {
        protected:
            Graph* mG;

            SwapFinding(Graph* g) : mG(g) {}

        public:
            /// \brief Struct for representing swaps.
            struct Swap {
                unsigned u;
                unsigned v;
            };

            typedef std::vector<std::pair<unsigned, unsigned>> RestrictionVector;
            typedef std::vector<Swap> SwapVector;

            /// \brief Given a std::vector of restrictions (edges in a graph), it returns
            /// a std::vector of swaps necessary to be done.
            virtual SwapVector findSwaps(RestrictionVector restrictions) = 0;
    };
}

#endif
