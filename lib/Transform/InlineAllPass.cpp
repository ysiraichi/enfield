#include "enfield/Transform/InlineAllPass.h"
#include "enfield/Transform/Utils.h"

using namespace efd;

// ==--------------- InlineAllPass ---------------==
uint8_t efd::InlineAllPass::ID = 0;

InlineAllPass::InlineAllPass(std::vector<std::string> basis) {
    mBasis = std::set<std::string>(basis.begin(), basis.end());
}

void InlineAllPass::appendInlinedInstructionsOfNode(Node::Ref node,
                                                    std::vector<Node::uRef>& inlined) {
    auto sPair = GetStatementPair(node);
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
        inlined.insert(inlined.end(),
                       std::make_move_iterator(innerInlinedInstr.begin()),
                       std::make_move_iterator(innerInlinedInstr.end()));
    } else {
        inlined.push_back(node->clone());
    }
}

std::vector<Node::uRef> InlineAllPass::getInlinedInstructionForGate(const std::string& gateName) {
    if (mGateInlinedInstructions.find(gateName) == mGateInlinedInstructions.end()) {
        // For each quantum operation `node` inside the gate `gateName`, we try to
        // inline it. Otherwise, we just clone it.
        std::vector<Node::uRef> inlinedInstructions;
        for (auto& node : *(mGateDeclarations[gateName]->getGOpList())) {
            appendInlinedInstructionsOfNode(node.get(), inlinedInstructions);
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
    bool changed = false;

    // We create an map entry for each gate within the `QModule`,
    // mapping its name to its declaration (`nullptr` if none).
    for (auto it = qmod->gates_begin(), end = qmod->gates_end(); it != end; ++it) {
        mGateDeclarations[(*it)->getId()->getVal()] = dynCast<NDGateDecl>(*it);
    }

    // Finally, we will replace the nodes only if we were able to find an
    // implementation for them. Otherwise, we do nothing.
    std::vector<Node::uRef> newStatements;
    newStatements.reserve(qmod->getNumberOfStmts());
    for (auto it = qmod->stmt_begin(), e = qmod->stmt_end(); it != e; ++it) {
        appendInlinedInstructionsOfNode(it->get(), newStatements);
    }

    qmod->clearStatements();
    qmod->insertStatementLast(std::move(newStatements));

    return changed;
}

InlineAllPass::uRef InlineAllPass::Create(std::vector<std::string> basis) {
    return uRef(new InlineAllPass(basis));
}
