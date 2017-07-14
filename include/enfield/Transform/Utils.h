#ifndef __EFD_UTILS_H__
#define __EFD_UTILS_H__

#include "enfield/Analysis/Nodes.h"
#include "enfield/Transform/QModule.h"

namespace efd {
    /// \brief Inserts a node after \p it.
    void InsertNodeAfter(Node::Iterator& it, Node::uRef node);
    /// \brief Inserts a node before \p it.
    void InsertNodeBefore(Node::Iterator& it, Node::uRef node);
    
    /// \brief If found, inlines the gate that \p qop calls.
    void InlineGate(QModule::Ref qmod, NDQOpGeneric::Ref qop);

    /// \brief Clones only the gate operations.
    std::vector<Node::sRef> CloneGOp(NDGateDecl::Ref gateDecl);

    /// \brief Replaces the node \p ref with the nodes \p nodes.
    void ReplaceNodes(Node::Ref ref, std::vector<Node::sRef> nodes);

    /// \brief Inserts a call to the swap gate, after \p prev.
    void InsertSwapAfter(Node::Ref prev, Node::Ref lhs, Node::Ref rhs);
    /// \brief Inserts a call to the swap gate, before \p prev.
    void InsertSwapBefore(Node::Ref prev, Node::Ref lhs, Node::Ref rhs);

    /// \brief Inserts a swap between \p lhs and \p rhs after \p prev.
    /// The cnot gate used can be specified by \p gate. If it is null,
    /// the default will be the CX gate.
    void InsertInlinedSwap(Node::Ref prev, Node::Ref lhs, Node::Ref rhs,
            NDId::Ref gateId = nullptr);

    /// \brief Applies the Hadammard gate on all qbits of this statement.
    void ReverseCNode(Node::Ref node);
}

#endif
