#ifndef __EFD_GATE_INLINE_H__
#define __EFD_GATE_INLINE_H__

#include "enfield/Analysis/Nodes.h"
#include "enfield/Analysis/QModule.h"

namespace efd {
    /// \brief If found, inlines the gate that \p qop calls.
    void InlineGate(QModule* qmod, NDQOpGeneric* qop);

    /// \brief Clones only the gate operations.
    std::vector<NodeRef> CloneGOp(NDGateDecl* gateDecl);

    /// \brief Replaces the node \p ref with the nodes \p nodes.
    void ReplaceNodes(NodeRef ref, std::vector<NodeRef> nodes);
}

#endif
