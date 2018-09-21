#include "enfield/Transform/InlineAllPass.h"
#include "enfield/Transform/Utils.h"
#include "enfield/Analysis/NodeVisitor.h"

using namespace efd;

// ==--------------- InlineAllVisitor ---------------==
namespace efd {
    class InlineAllVisitor : public NodeVisitor {
        private:
            std::set<std::string> mBasis;

        public:
            std::vector<NDQOp::Ref> mInlineVector;

            InlineAllVisitor(std::set<std::string> basis) : mBasis(basis) {}

            void visit(NDQOpGen::Ref ref) override;
            void visit(NDIfStmt::Ref ref) override;
    };
}

void InlineAllVisitor::visit(NDQOpGen::Ref ref) {
    if (mBasis.find(ref->getOperation()) == mBasis.end()) {
        mInlineVector.push_back(ref);
    }
}

void InlineAllVisitor::visit(NDIfStmt::Ref ref) {
    ref->getQOp()->apply(this);
}

// ==--------------- InlineAllPass ---------------==
uint8_t efd::InlineAllPass::ID = 0;

InlineAllPass::InlineAllPass(std::vector<std::string> basis) {
    mBasis = std::set<std::string>(basis.begin(), basis.end());
}

std::vector<Node::uRef> InlineAllPass::getInlinedInstructionForGate(const std::string& gateName) {
    if (mGateInlinedInstructions.find(gateName) == mGateInlinedInstructions.end()) {
        // For each quantum operation `node` inside the gate `gateName`, we try to
        // inline it. Otherwise, we just clone it.
        std::vector<Node::uRef> inlinedInstructions;

        for (auto& node : *(mGateDeclarations[gateName]->getGOpList())) {
            auto sPair = GetStatementPair(node.get());
            auto innerGateName = sPair.second->getOperation();

            NDGateDecl::Ref innerGateDecl = nullptr;

            if (mGateDeclarations.find(innerGateName) != mGateDeclarations.end()) {
                innerGateDecl = mGateDeclarations[innerGateName];
            }

            // We will inline `node` iff it isn't listed as one of the basis
            // gates, and we can find an implementation.
            if (mBasis.find(innerGateName) == mBasis.end() && innerGateDecl != nullptr) {
                auto argMap = CreateInlineArgMap(innerGateDecl, sPair.second);
                auto innerInlinedInstr = getInlinedInstructionForGate(innerGateName);

                ReplaceInlineArgMap(argMap, innerInlinedInstr, sPair.first);
                inlinedInstructions.insert(inlinedInstructions.end(),
                                           std::make_move_iterator(innerInlinedInstr.begin()),
                                           std::make_move_iterator(innerInlinedInstr.end()));
            } else {
                inlinedInstructions.push_back(node->clone());
            }
        }

        // Saving the inlined instructions in a cache for future use.
        mGateInlinedInstructions[gateName] = std::move(inlinedInstructions);
    }

    // Finally, we clone the inlined instructions, and return them.
    std::vector<Node::uRef> inlinedInstructions;

    for (auto& instr : mGateInlinedInstructions[gateName]) {
        inlinedInstructions.push_back(instr->clone());
    }

    return inlinedInstructions;
}

bool InlineAllPass::run(QModule::Ref qmod) {
    InlineAllVisitor visitor(mBasis);

    mGateDeclarations.clear();
    mGateInlinedInstructions.clear();

    bool changed = false;

    // First, we need to get all the nodes that should be replaced.
    for (auto it = qmod->stmt_begin(), e = qmod->stmt_end(); it != e; ++it) {
        (*it)->apply(&visitor);
    }

    // Then, we create an map entry for each gate within the `QModule`,
    // mapping its name to its declaration (`nullptr` if none).
    for (auto it = qmod->gates_begin(), end = qmod->gates_end(); it != end; ++it) {
        mGateDeclarations[(*it)->getId()->getVal()] = dynCast<NDGateDecl>(*it);
    }

    // Finally, we will replace the nodes only if we were able to find an
    // implementation for them. Otherwise, we do nothing.
    for (auto node : visitor.mInlineVector) {
        auto gateName = node->getOperation();
        auto gateDecl = mGateDeclarations[gateName];

        if (gateDecl != nullptr) {
            auto inlinedInstructions = getInlinedInstructionForGate(gateName);
            auto argMap = CreateInlineArgMap(gateDecl, node);

            auto ifstmt = dynCast<NDIfStmt>(node->getParent());
            ReplaceInlineArgMap(argMap, inlinedInstructions, ifstmt);
            qmod->replaceStatement((ifstmt == nullptr) ? (Node::Ref) node : (Node::Ref)ifstmt,
                                   std::move(inlinedInstructions));
            changed = true;
        }
    }

    return changed;
}

InlineAllPass::uRef InlineAllPass::Create(std::vector<std::string> basis) {
    return uRef(new InlineAllPass(basis));
}
