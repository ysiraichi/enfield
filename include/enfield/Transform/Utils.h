#ifndef __EFD_UTILS_H__
#define __EFD_UTILS_H__

#include "enfield/Analysis/Nodes.h"
#include "enfield/Transform/QModule.h"

namespace efd {
    /// \brief Inserts a node after \p it.
    void InsertNodeAfter(Node::Iterator& it, NodeRef node);
    /// \brief Inserts a node before \p it.
    void InsertNodeBefore(Node::Iterator& it, NodeRef node);
    
    /// \brief If found, inlines the gate that \p qop calls.
    void InlineGate(QModule* qmod, NDQOpGeneric* qop);

    /// \brief Clones only the gate operations.
    std::vector<NodeRef> CloneGOp(NDGateDecl* gateDecl);

    /// \brief Replaces the node \p ref with the nodes \p nodes.
    void ReplaceNodes(NodeRef ref, std::vector<NodeRef> nodes);

    /// \brief Inserts a call to the swap gate, after \p prev.
    void InsertSwapAfter(NodeRef prev, NodeRef lhs, NodeRef rhs);
    /// \brief Inserts a call to the swap gate, before \p prev.
    void InsertSwapBefore(NodeRef prev, NodeRef lhs, NodeRef rhs);

    /// \brief Inserts a swap between \p lhs and \p rhs after \p prev.
    /// The cnot gate used can be specified by \p gate. If it is null,
    /// the default will be the CX gate.
    void InsertInlinedSwap(NodeRef prev, NodeRef lhs, NodeRef rhs, NDId* gateId = nullptr);

    /// \brief Applies the Hadammard gate on all qbits of this statement.
    void ReverseCNode(NodeRef node);
}

#endif
