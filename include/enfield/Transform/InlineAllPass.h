#ifndef __EFD_INLINE_ALL_PASS_H__
#define __EFD_INLINE_ALL_PASS_H__

#include "enfield/Transform/Pass.h"
#include "enfield/Transform/QModule.h"

#include <set>

namespace efd {
    class InlineAllPass : public PassT<void> {
        public:
            typedef InlineAllPass* Ref;
            typedef std::unique_ptr<InlineAllPass> uRef;

            static uint8_t ID;

        private:
            std::set<std::string> mBasis;
            std::unordered_map<std::string, NDGateDecl::Ref> mGateDeclarations;
            std::unordered_map<std::string, std::vector<Node::uRef>> mGateInlinedInstructions;

            void appendInlinedInstructionsOfNode(Node::Ref node,
                                                 std::vector<Node::uRef>& inlined);
            std::vector<Node::uRef> getInlinedInstructionForGate(const std::string& gateName);
            
        public:
            InlineAllPass(std::vector<std::string> basis = std::vector<std::string>());

            bool run(QModule::Ref qmod) override;

            /// \brief Creates an instance of this pass.
            static uRef Create(std::vector<std::string> basis = 
                    std::vector<std::string>());
    };
}

#endif
