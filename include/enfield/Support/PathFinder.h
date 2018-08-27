#ifndef __EFD_PATH_FINDER_H__
#define __EFD_PATH_FINDER_H__

#include "enfield/Support/Graph.h"

namespace efd {
    /// \brief Interface for finding the path between two vertices.
    class PathFinder {
        public:
            typedef PathFinder* Ref;
            typedef std::shared_ptr<PathFinder> sRef;

            virtual ~PathFinder() = default;

            /// \brief Searches for a path from \p u to \p v in the graph \p g.
            ///
            /// This function is to be implemented by the concrete classes. It should
            /// return the path from \p u to \p v (including them both).
            virtual std::vector<uint32_t> find(Graph::Ref g, uint32_t u, uint32_t v) = 0;
    };
}

#endif
