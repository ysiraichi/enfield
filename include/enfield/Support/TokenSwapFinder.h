#ifndef __EFD_TOKEN_SWAP_FINDER_H__
#define __EFD_TOKEN_SWAP_FINDER_H__

#include "enfield/Support/Graph.h"
#include "enfield/Support/Defs.h"

namespace efd {
    /// \brief Interface for solving the Token Swap Problem.
    class TokenSwapFinder {
        public:
            typedef TokenSwapFinder* Ref;
            typedef std::unique_ptr<TokenSwapFinder> uRef;

        protected:
            Graph::Ref mG;
            TokenSwapFinder();

            void checkGraphSet();
            virtual void preprocess() = 0;
            virtual SwapSeq findImpl(const Assign& from, const Assign& to) = 0;

        public:
            /// \brief Sets the `Graph`.
            void setGraph(Graph::Ref graph);
            /// \brief Finds a swap sequence to reach \p to from \p from.
            SwapSeq find(const Assign& from, const Assign& to);
    };
}

#endif
