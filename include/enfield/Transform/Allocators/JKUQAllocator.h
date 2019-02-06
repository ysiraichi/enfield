#ifndef __EFD_JKU_QALLOCATOR_H__
#define __EFD_JKU_QALLOCATOR_H__

#include "enfield/Transform/Allocators/QbitAllocator.h"
#include "enfield/Transform/DependencyBuilderPass.h"
#include "enfield/Transform/LayersBuilderPass.h"
#include "enfield/Support/BFSPathFinder.h"
#include "enfield/Support/Defs.h"

#include <queue>

namespace efd {
namespace jku {
    struct AStarNode;
    struct ExpandNodeState;
}

    /// \brief JKU QAllocator.
    ///
    /// Implemented from Zulehner et. al.:
    /// An Efficient Methodology for Mapping Quantum Circuits to the
    /// IBM QX Architectures
    ///
    /// Main C implementation can be found at:
    /// http://iic.jku.at/eda/research/ibm_qx_mapping/
    class JKUQAllocator : public QbitAllocator {
        public:
            typedef JKUQAllocator* Ref;
            typedef std::unique_ptr<JKUQAllocator> uRef;

        private:
            std::vector<std::vector<uint32_t>> mTable;
            DependencyBuilder mDBuilder;
            BFSPathFinder mBFS;

            void buildCostTable();
            void expandNodeRecursively(const jku::AStarNode& aNode,
                                       uint32_t i,
                                       jku::ExpandNodeState& state);

            jku::AStarNode astar(std::queue<uint32_t>& cnotLayersIdQ,
                                 const Layers& layers,
                                 uint32_t i,
                                 Mapping& mapping,
                                 InverseMap& inverse);
        public:
            JKUQAllocator(ArchGraph::sRef archGraph);

            Mapping allocate(QModule::Ref qmod) override;
            static uRef Create(ArchGraph::sRef archGraph);
    };
}

#endif
