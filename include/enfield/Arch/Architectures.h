#ifndef __EFD_ARCHITECTURES_H__
#define __EFD_ARCHITECTURES_H__

#include "enfield/Arch/ArchGraph.h"

namespace efd {

#define EFD_ARCHITECTURE(_Name_, _QbitNum_) \
    class Arch##_Name_ : public ArchGraph {\
        private:\
            Arch##_Name_();\
        public:\
            static std::unique_ptr<Arch##_Name_> Create() {\
                return std::unique_ptr<Arch##_Name_>(new Arch##_Name_());\
            }\
    };

// Defines the undefined macros.
#include "enfield/Arch/Defs.h"

// From a previous configuration file '.def', by including it, it'll generate
// the declaration of the architectures.
#include "enfield/Arch/IBMQX2.def"

// Undefines all macros.
#include "enfield/Arch/Undefs.h"

}

#endif
