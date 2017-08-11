#ifndef __EFD_PATH_FINDER_H__
#define __EFD_PATH_FINDER_H__

#include "enfield/Support/Graph.h"

namespace efd {
    /// \brief Interface for finding the path between two vertices.
    class PathFinder {
        public:
            typedef PathFinder* Ref;
            typedef std::shared_ptr<PathFinder> sRef;

        protected:
            Graph::sRef mG;

            PathFinder(Graph::sRef g) : mG(g) {}

        public:
            /// \brief Searches for a path from \p u to \p v in the graph \em mG.
            ///
            /// This function is to be implemented by the concrete classes. It should
            /// return the path from \p u to \p v (including them both).
            virtual std::vector<unsigned> find(unsigned u, unsigned v) = 0;
    };
}

#endif
