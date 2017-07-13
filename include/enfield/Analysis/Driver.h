#ifndef __EFD_DRIVER_H__
#define __EFD_DRIVER_H__

#include "enfield/Analysis/Nodes.h"

#include <iostream>
#include <string>

namespace efd {
    struct ASTWrapper {
        // The parser input
        std::string mFile;
        std::string mPath;

        // The parser output
        Node::Ref mAST;
        // Has parsed standard library
        bool mStdLibParsed;
    };

    /// \brief Parse \p filename at \p path.
    Node::uRef ParseFile(std::string filename, std::string path = "./", bool forceStdLib = true);
    /// \brief Parse the string \p program.
    Node::uRef ParseString(std::string program, bool forceStdLib = true);
};

#endif
