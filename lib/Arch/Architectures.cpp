#include "enfield/Arch/Architectures.h"

#define EFD_ARCHITECTURE(_Name_, _QbitNum_) \
    efd::Arch##_Name_::Arch##_Name_() : ArchGraph(_QbitNum_) {\
        unsigned u, v;

#define EFD_REG(_QReg_, _Size_) \
        for (unsigned i = 0; i < _Size_; ++i)\
            this->putVertex(std::string(#_QReg_"[" + std::to_string(i) + "]"));

#define EFD_COUPLING(_QReg_, _U_, _V_)\
        u = this->getUId(#_QReg_"["#_U_"]");\
        v = this->getUId(#_QReg_"["#_V_"]");\
        this->putEdge(u, v);

#define EFD_ARCHITECTURE_END\
        this->buildReverseGraph();\
    }

// Defines the undefined macros.
#include "enfield/Arch/Defs.h"

// From a previous configuration file '.def', by including it, it'll generate
// the implementation of the class.
#include "enfield/Arch/IBMQX2.def"

// Undefines all macros.
#include "enfield/Arch/Undefs.h"
