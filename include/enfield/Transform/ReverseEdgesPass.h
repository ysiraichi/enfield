#ifndef __EFD_REVERSE_EDGES_PASS_H__
#define __EFD_REVERSE_EDGES_PASS_H__

#include "enfield/Pass.h"
#include "enfield/Transform/QModule.h"
#include "enfield/Transform/DependencyBuilderPass.h"
#include "enfield/Arch/ArchGraph.h"

namespace efd {
    /// \brief Pass that reverses the edges accordingly with the architecture Graph.
    class ReverseEdgesPass : public Pass {
        public:
            typedef ReverseEdgesPass* Ref;
            typedef std::unique_ptr<ReverseEdgesPass> uRef;

        private:
            ArchGraph::sRef mG;
            QModule::sRef mMod;

            DependencyBuilderPass::uRef mDepPass;

            ReverseEdgesPass(QModule::sRef qmod, ArchGraph::sRef graph);

            void initImpl(bool force) override;

        public:
            void visit(NDQOpCX::Ref ref) override;
            void visit(NDQOpGeneric::Ref ref) override;

            bool doesInvalidatesModule() const override;

            /// \brief Create an instance of this class.
            static uRef Create(QModule::sRef qmod, ArchGraph::sRef graph);
    };
}

#endif
