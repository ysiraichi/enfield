#ifndef __EFD_TOKEN_SWAP_FINDER_H__
#define __EFD_TOKEN_SWAP_FINDER_H__

#include "enfield/Support/Graph.h"
#include "enfield/Support/Defs.h"

namespace efd {
    class TokenSwapFinder {
        protected:
            Graph::sRef mG;
            TokenSwapFinder(Graph::sRef graph) : mG(graph) {}

        public:
            /// \brief Finds a swap sequence to reach \p to from \p from.
            virtual SwapSeq find(Assign from, Assign to) = 0;
            /// \brief Finds a swap sequence to reach \p to from \p from in \p graph.
            virtual SwapSeq find(Graph::Ref graph, Assign from, Assign to) = 0;
    };
}

#endif
