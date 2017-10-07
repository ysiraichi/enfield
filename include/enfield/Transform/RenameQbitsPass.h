#ifndef __EFD_RENAME_QBIT_PASS_H__
#define __EFD_RENAME_QBIT_PASS_H__

#include "enfield/Transform/Pass.h"
#include "enfield/Transform/QModule.h"
#include "enfield/Analysis/Nodes.h"

#include <unordered_map>
#include <string>

namespace efd {
    /// \brief Renames all the qbits according to the map from the constructor.
    class RenameQbitPass : public PassT<void> {
        public:
            typedef RenameQbitPass* Ref;
            typedef std::unique_ptr<RenameQbitPass> uRef;

            typedef std::unordered_map<std::string, Node::Ref> ArchMap;

            static uint8_t ID;

        private:
            ArchMap mAMap;

            RenameQbitPass(ArchMap map);

        public:
            bool run(QModule::Ref qmod) override;

            /// \brief Creates a new instance of this pass.
            static RenameQbitPass::uRef Create(ArchMap map);
    };
}

#endif
