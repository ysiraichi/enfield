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

    /// \brief Parse \p filename at \p path.
    NodeRef ParseFile(std::string filename, std::string path = "./");
    /// \brief Parse the string \p program.
    NodeRef ParseString(std::string program);
};

#endif
