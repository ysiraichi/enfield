#ifndef __EFD_REVERSE_EDGES_PASS_H__
#define __EFD_REVERSE_EDGES_PASS_H__

#include "enfield/Transform/Pass.h"
#include "enfield/Transform/QModule.h"
#include "enfield/Arch/ArchGraph.h"

namespace efd {
    /// \brief Pass that reverses the edges accordingly with the architecture Graph.
    class ReverseEdgesPass : public PassT<void> {
        public:
            typedef ReverseEdgesPass* Ref;
            typedef std::unique_ptr<ReverseEdgesPass> uRef;

            static uint8_t ID;

        private:
            ArchGraph::sRef mG;

            ReverseEdgesPass(ArchGraph::sRef graph);

        public:
            bool run(QModule::Ref qmod) override;

            /// \brief Create an instance of this class.
            static uRef Create(ArchGraph::sRef graph);
    };
}

#endif
