#include "enfield/Arch/Architectures.h"
#include "enfield/Analysis/Nodes.h"

#define EFD_ARCHITECTURE(_Name_, _QbitNum_) \
    efd::Arch##_Name_::Arch##_Name_() : ArchGraph(_QbitNum_, false) {\
        unsigned u, v;

#define EFD_REG(_QReg_, _Size_) \
        this->putReg(#_QReg_, #_Size_);\
        NodeRef ndID = NDId::Create(#_QReg_);\
        for (unsigned i = 0; i < _Size_; ++i) {\
            NodeRef ndN = NDInt::Create(std::to_string(i));\
            this->putVertex(NDIdRef::Create(ndID->clone(), ndN));\
        }

#define EFD_COUPLING(_QReg_, _U_, _V_)\
        u = this->getUId(#_QReg_"["#_U_"]");\
        v = this->getUId(#_QReg_"["#_V_"]");\
        this->putEdge(u, v);

#define EFD_ARCHITECTURE_END\
    }

// Defines the undefined macros.
#include "enfield/Arch/Defs.h"

// From a previous configuration file '.def', by including it, it'll generate
// the implementation of the class.
#include "enfield/Arch/IBMQX2.def"

// Undefines all macros.
#include "enfield/Arch/Undefs.h"
