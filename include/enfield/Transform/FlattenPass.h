#ifndef __EFD_FLATTEN_PASS_H__
#define __EFD_FLATTEN_PASS_H__

#include "enfield/Transform/Pass.h"
#include "enfield/Transform/QModule.h"

namespace efd {
    /// \brief Flattens the given QModule.
    ///
    /// It will expand all implicit operations. 
    /// e.g.: qreg b[2]; x b; -> qreg b[2]; x b[0]; x b[1];
    class FlattenPass : public PassT<void> {
        public:
            typedef FlattenPass* Ref;
            typedef std::unique_ptr<FlattenPass> uRef;

        private:
            FlattenPass();

            /// \brief Replaces \p ref by the nodes in \p nodes.
            void replace(Node::Ref ref, std::vector<Node::uRef> nodes);

        public:
            void run(QModule::Ref qmod) override;

            static uRef Create();
    };
}

#endif
