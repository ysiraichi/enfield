#ifndef __EFD_DEPENDENCY_GRAPH_BUILDER_PASS_H__
#define __EFD_DEPENDENCY_GRAPH_BUILDER_PASS_H__

#include "enfield/Transform/Pass.h"
#include "enfield/Support/WeightedGraph.h"
#include <map>

namespace efd {
    typedef WeightedGraph<uint32_t> DependencyGraph;

    /// \brief Constructs the dependency graph for a \p QModule.
    ///
    /// A dependency graph is an weighted directed graph that has an edge
    /// from 'a' to 'b' if, and only if, there is a dependency from 'a' to
    /// 'b' in the program. It's weight represents the frequency.
    class DependencyGraphBuilderPass : public PassT<DependencyGraph::sRef> {
        public:
            typedef DependencyGraphBuilderPass* Ref;
            typedef std::unique_ptr<DependencyGraphBuilderPass> uRef;

            static uint8_t ID;

            bool run(QModule* qmod) override;
            static uRef Create();
    };
}

#endif
