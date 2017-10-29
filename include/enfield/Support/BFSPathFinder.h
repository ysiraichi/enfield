#ifndef __EFD_BFS_PATH_FINDER_H__
#define __EFD_BFS_PATH_FINDER_H__

#include "enfield/Support/PathFinder.h"

namespace efd {
    /// \brief Finds swaps for one restriction only.
    class BFSPathFinder : public PathFinder {
        public:
            typedef BFSPathFinder* Ref;
            typedef std::unique_ptr<BFSPathFinder> uRef;
            typedef std::shared_ptr<BFSPathFinder> sRef;

            BFSPathFinder();

            std::vector<uint32_t> find(Graph::Ref g, uint32_t u, uint32_t v) override;

            /// \brief Creates one instance of this finder.
            static uRef Create();
    };
}

#endif
