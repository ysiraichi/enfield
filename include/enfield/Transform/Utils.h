#ifndef __EFD_UTILS_H__
#define __EFD_UTILS_H__

#include "enfield/Analysis/Nodes.h"
#include "enfield/Transform/QModule.h"

namespace efd {
    /// \brief If found, inlines the gate that \p qop calls.
    void InlineGate(QModule::Ref qmod, NDQOpGeneric::Ref qop);
    /// \brief Applies the Hadammard gate on all qbits of this statement.
    void ReverseCNode(QModule::Ref ref, Node::Ref node);
    /// \brief Processes the \p root node, and transform the entire AST into
    /// a QModule.
    void ProcessAST(QModule::Ref qmod, Node::Ref root);

    /// \brief Returns a vector with the intrinsic gates implementation.
    std::vector<NDGateSign::uRef> GetIntrinsicGates();

    /// \brief Creates a call to the intrinsic swap function.
    NDQOpGeneric::uRef CreateISwap(Node::uRef lhs, Node::uRef rhs);
    /// \brief Creates a call to the intrinsic long cnot function.
    NDQOpGeneric::uRef CreateILongCX(Node::uRef lhs, Node::uRef middle, Node::uRef rhs);
    /// \brief Creates a call to the intrinsic reversal cnot function.
    NDQOpGeneric::uRef CreateIRevCX(Node::uRef lhs, Node::uRef rhs);
}

#endif
