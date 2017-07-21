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
}

#endif
