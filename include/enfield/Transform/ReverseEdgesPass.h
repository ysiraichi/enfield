#ifndef __EFD_REVERSE_EDGES_PASS_H__
#define __EFD_REVERSE_EDGES_PASS_H__

#include "enfield/Pass.h"
#include "enfield/Transform/QModule.h"
#include "enfield/Transform/DependencyBuilderPass.h"
#include "enfield/Arch/ArchGraph.h"

namespace efd {
    /// \brief Pass that reverses the edges accordingly with the architecture Graph.
    class ReverseEdgesPass : public Pass {
        private:
            ArchGraph* mG;
            QModule* mMod;

            DependencyBuilderPass* mDepPass;

            ReverseEdgesPass(QModule* qmod, ArchGraph* graph);

            void initImpl(bool force) override;

        public:
            void visit(NDQOpCX* ref) override;
            void visit(NDQOpGeneric* ref) override;

            bool doesInvalidatesModule() const override;

            /// \brief Create an instance of this class.
            static ReverseEdgesPass* Create(QModule* qmod, ArchGraph* graph);
    };
}

#endif
