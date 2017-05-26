#ifndef __EFD_FLATTEN_PASS_H__
#define __EFD_FLATTEN_PASS_H__

#include "enfield/Pass.h"
#include "enfield/Transform/QModule.h"

namespace efd {
    /// \brief Flattens the given QModule.
    ///
    /// It will expand all implicit operations. 
    /// e.g.: qreg b[2]; x b; -> qreg b[2]; x b[0]; x b[1];
    class FlattenPass : public Pass {
        private:
            QModule* mMod;
            std::vector<NodeRef> mIfNewNodes;

            FlattenPass(QModule* qmod);

            /// \brief Returns true if \p ref is an IdRef.
            bool isIdRef(NodeRef ref);
            /// \brief Returns true if \p ref is an Id.
            bool isId(NodeRef ref);

            /// \brief Gets the declaration node from an Id node.
            NDDecl* getDeclFromId(NodeRef ref);
            /// \brief Creates \p max NDIdRef's related to that Id. If \p max
            /// is 0, then create all of them.
            std::vector<NodeRef> toIdRef(NodeRef ref, unsigned max = 0);
            /// \brief Replaces \p ref by the nodes in \p nodes.
            void replace(NodeRef ref, std::vector<NodeRef> nodes);

            /// \brief Returns the size of the declaration of this Id node.
            unsigned getSize(NodeRef ref);
            /// \brief Returns a list with all IdRef's possible of all QArgs.
            std::vector<std::vector<NodeRef>> getFlattenedOpsArgs(NodeRef ref);
            /// \brief Returns true if all the childrem are IdRef node.
            bool isChildremIdRef(NodeRef ref);

        public:
            void visit(NDQOpCX* ref) override;
            void visit(NDQOpGeneric* ref) override;
            void visit(NDIfStmt* ref) override;

            void initImpl() override;

            bool doesInvalidatesModule() const override;

            static FlattenPass* Create(QModule* qmod);
    };
}

#endif
