#ifndef __EFD_DRIVER_H__
#define __EFD_DRIVER_H__

#include "enfield/Analysis/Nodes.h"

#include <iostream>
#include <string>

namespace efd {
    struct ASTWrapper {
        std::string mFile;
        std::string mPath;
        NodeRef mAST;
    };

    NodeRef ParseFile(std::string filename, std::string path = "./");
    NodeRef ParseString(std::string program);
};

#endif
