#ifndef __EFD_RENAME_QBIT_PASS_H__
#define __EFD_RENAME_QBIT_PASS_H__

#include "enfield/Pass.h"

#include <unordered_map>
#include <string>

namespace efd {
    class QbitToNumberPass;

    /// \brief Renames all the qbits according to the map from the constructor.
    class RenameQbitPass : public Pass {
        public:
            typedef std::unordered_map<std::string, NodeRef> ArchMap;

        private:
            ArchMap mAMap;

            RenameQbitPass(ArchMap map);

            /// \brief Getsh the node associated with the old node (that is currently
            /// inside)
            NodeRef getNodeFromOld(NodeRef old);
            /// \brief Returns true if this call is a call to the swap gate.
            bool isSwapGate(NDQOpGeneric* ref);

        public:
            void visit(NDQOpMeasure* ref) override;
            void visit(NDQOpReset* ref) override;
            void visit(NDQOpU* ref) override;
            void visit(NDQOpCX* ref) override;
            void visit(NDQOpBarrier* ref) override;
            void visit(NDQOpGeneric* ref) override;
            void visit(NDList* ref) override;

            bool doesInvalidatesModule() const override;

            /// \brief Creates a new instance of this pass.
            static RenameQbitPass* Create(ArchMap map);
    };
}

#endif
