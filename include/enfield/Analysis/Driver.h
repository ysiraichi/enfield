#ifndef __EFD_DRIVER_H__
#define __EFD_DRIVER_H__

#include "enfield/Analysis/Nodes.h"

#include <string>

namespace efd {
    struct ASTWrapper {
        ASTWrapper(std::string filename);

        std::string mFilename;
        std::shared_ptr<Node::NodeRef> mAST;
    };

    int Parse(ASTWrapper& ast);
};

#endif
