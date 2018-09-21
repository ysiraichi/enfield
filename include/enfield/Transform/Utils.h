#ifndef __EFD_UTILS_H__
#define __EFD_UTILS_H__

#include "enfield/Analysis/Nodes.h"
#include "enfield/Transform/QModule.h"

#include <map>

namespace efd {
    using GateWeightMap = std::map<std::string, uint32_t>;
    using GateNameVector = std::vector<std::string>;
    using StatementPair = std::pair<NDIfStmt::Ref, NDQOp::Ref>;
    using InlineArgMap = std::unordered_map<std::string, Node::Ref>;

    /// \brief Extracts only the gate names to a separate vector.
    GateNameVector ExtractGateNames(const GateWeightMap& map);
    /// \brief Breakdowns the \p node into \em NDIfStmt and \em NDQOp pair.
    StatementPair GetStatementPair(const Node::Ref node);

    /// \brief Create an \em InlineArgMap for \p call and \p gateDecl.
    ///
    /// Creates a mapping from the qubits used in \p gateDecl to the qubits used
    /// in \p call. It prepares for inlining.
    InlineArgMap CreateInlineArgMap(NDGateDecl::Ref gateDecl, NDQOp::Ref call);
    /// \brief Replace the qubits used in \p inlinedInstructions according to
    /// the mapping \p argMap.
    void ReplaceInlineArgMap(const InlineArgMap& argMap,
                             std::vector<Node::uRef>& inlinedInstructions,
                             NDIfStmt::Ref ifstmt = nullptr);
    /// \brief If found, inlines the gate that \p qop calls.
    void InlineGate(QModule::Ref qmod, NDQOp::Ref qop);
    /// \brief Processes the \p root node, and transform the entire AST into
    /// a QModule.
    void ProcessAST(QModule::Ref qmod, Node::Ref root);

    /// \brief Returns a vector with the intrinsic gates implementation.
    std::vector<NDGateSign::uRef> GetIntrinsicGates();

    /// \brief Creates a call to the intrinsic swap function.
    NDQOp::uRef CreateISwap(Node::uRef lhs, Node::uRef rhs);
    /// \brief Creates a call to the intrinsic long cnot function.
    NDQOp::uRef CreateILongCX(Node::uRef lhs, Node::uRef middle, Node::uRef rhs);
    /// \brief Creates a call to the intrinsic reversal cnot function.
    NDQOp::uRef CreateIRevCX(Node::uRef lhs, Node::uRef rhs);

    /// \brief Returns whether \p ref is a CNOT gate.
    ///
    /// This is used because there may be gates like the standard library 'cx'.
    bool IsCNOTGateCall(Node::Ref ref);
    /// \brief Returns whether \p ref is (or should be) an intrinsic gate.
    bool IsIntrinsicGateCall(Node::Ref ref);
    /// \brief Gets the NDQOpGen::IntrinsicKind, given a Node \p ref.
    ///
    /// This function should be only called if \p ref is really an intrinsic function.
    /// Otherwise, it will crash.
    NDQOpGen::IntrinsicKind GetIntrinsicKind(Node::Ref ref);
    /// \brief Creates an intrinsic gate of kind \p kind and arguments \p args.
    NDQOp::uRef CreateIntrinsicGate(NDQOpGen::IntrinsicKind kind,
                                    std::vector<Node::uRef> qargs);
}

#endif
