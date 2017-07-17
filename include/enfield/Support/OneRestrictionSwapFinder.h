#ifndef __EFD_ONE_RESTRICTION_SWAP_FINDER_H__
#define __EFD_ONE_RESTRICTION_SWAP_FINDER_H__

#include "enfield/Support/SwapFinder.h"

namespace efd {
    /// \brief Finds swaps for one restriction only.
    class OneRestrictionSwapFinder : public SwapFinder {
        public:
            typedef OneRestrictionSwapFinder* Ref;
            typedef std::unique_ptr<OneRestrictionSwapFinder> uRef;
            typedef std::shared_ptr<OneRestrictionSwapFinder> sRef;

        protected:
            OneRestrictionSwapFinder(Graph::sRef g);

            /// \brief Returns the path from r.From to r.To.
            std::vector<unsigned> getPath(Rest r);
            /// \brief Generates the sequence of swaps, based on the path.
            SwapVector generateFromPath(std::vector<unsigned> path);

        public:
            SwapVector findSwaps(RestrictionVector restrictions) override;

            /// \brief Creates one instance of this finder.
            static uRef Create(Graph::sRef g);
    };
}

#endif
