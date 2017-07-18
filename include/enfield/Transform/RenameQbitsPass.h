#ifndef __EFD_RENAME_QBIT_PASS_H__
#define __EFD_RENAME_QBIT_PASS_H__

#include "enfield/Pass.h"
#include "enfield/Analysis/Nodes.h"

#include <unordered_map>
#include <string>

namespace efd {
    class QbitToNumberPass;

    /// \brief Renames all the qbits according to the map from the constructor.
    class RenameQbitPass : public Pass {
        public:
            typedef RenameQbitPass* Ref;
            typedef std::unique_ptr<RenameQbitPass> uRef;

            typedef std::unordered_map<std::string, Node::Ref> ArchMap;

        private:
            ArchMap mAMap;

            RenameQbitPass(ArchMap map);

            /// \brief Gets the node associated with the old node (that is currently
            /// inside)
            Node::uRef getNodeFromOld(Node::Ref old);
            /// \brief Returns true if this call is a call to the swap gate.
            bool isSwapGate(NDQOpGeneric::Ref ref);

        public:
            void visit(NDQOpMeasure::Ref ref) override;
            void visit(NDQOpReset::Ref ref) override;
            void visit(NDQOpU::Ref ref) override;
            void visit(NDQOpCX::Ref ref) override;
            void visit(NDQOpBarrier::Ref ref) override;
            void visit(NDQOpGeneric::Ref ref) override;
            void visit(NDList::Ref ref) override;

            bool doesInvalidatesModule() const override;

            /// \brief Creates a new instance of this pass.
            static RenameQbitPass::uRef Create(ArchMap map);
    };
}

#endif
