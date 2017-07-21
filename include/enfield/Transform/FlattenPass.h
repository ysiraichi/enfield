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
        public:
            typedef FlattenPass* Ref;
            typedef std::unique_ptr<FlattenPass> uRef;

        private:
            QModule::sRef mMod;
            std::vector<Node::sRef> mIfNewNodes;

            FlattenPass(QModule::sRef qmod);

            /// \brief Returns true if \p ref is an IdRef.
            bool isIdRef(Node::Ref ref);
            /// \brief Returns true if \p ref is an Id.
            bool isId(Node::Ref ref);

            /// \brief Gets the declaration node from an Id node.
            NDRegDecl::Ref getDeclFromId(Node::Ref ref);
            /// \brief Creates \p max NDIdRef's related to that Id. If \p max
            /// is 0, then create all of them.
            std::vector<NDIdRef::uRef> toIdRef(Node::Ref ref, unsigned max = 0);
            /// \brief Replaces \p ref by the nodes in \p nodes.
            void replace(Node::Ref ref, std::vector<Node::uRef> nodes);

            /// \brief Returns the size of the declaration of this Id node.
            unsigned getSize(Node::Ref ref);
            /// \brief Returns a list with all IdRef's possible of all QArgs.
            std::vector<std::vector<NDIdRef::uRef>> getFlattenedOpsArgs(Node::Ref ref);
            /// \brief Returns true if all the childrem are IdRef node.
            bool isChildremIdRef(Node::Ref ref);

        public:
            void visit(NDQOpBarrier::Ref ref) override;
            void visit(NDQOpMeasure::Ref ref) override;
            void visit(NDQOpReset::Ref ref) override;
            void visit(NDQOpCX::Ref ref) override;
            void visit(NDQOpGeneric::Ref ref) override;

            void initImpl(bool force) override;

            bool doesInvalidatesModule() const override;

            static uRef Create(QModule::sRef qmod);
    };
}

#endif
