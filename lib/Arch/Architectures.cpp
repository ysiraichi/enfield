#include "enfield/Arch/Architectures.h"
#include "enfield/Analysis/Nodes.h"
#include "enfield/Support/RTTI.h"

namespace efd {
    typedef std::shared_ptr<ArchRegistry> ArchRegistryPtr;

#define EFD_ARCHITECTURE(_Name_, _QbitNum_) \
    class Arch##_Name_ : public ArchGraph {\
        public:\
            static std::unique_ptr<Arch##_Name_> Create() {\
                return std::unique_ptr<Arch##_Name_>(new Arch##_Name_());\
            }\
        private:\
            Arch##_Name_() : ArchGraph(_QbitNum_, false) {\
                uint32_t u, v;

#define EFD_REG(_QReg_, _Size_) \
                this->putReg(#_QReg_, #_Size_);\
                auto ndID = NDId::Create(#_QReg_);\
                for (uint32_t i = 0; i < _Size_; ++i) {\
                    auto ndN = NDInt::Create(std::to_string(i));\
                    auto ndIDCpy = dynCast<NDId>(ndID->clone().release());\
                    this->putVertex(NDIdRef::Create\
                            (NDId::uRef(ndIDCpy), std::move(ndN)));\
                }

#define EFD_COUPLING(_QReg_, _U_, _V_) \
                u = this->getUId(#_QReg_"["#_U_"]");\
                v = this->getUId(#_QReg_"["#_V_"]");\
                this->putEdge(u, v, 1);

#define EFD_ARCHITECTURE_END \
            }\
    };

// From a previous configuration file '.def', by including it, it'll generate
// the declaration of the architectures.
#include "enfield/Arch/IBMQX2.def"
#include "enfield/Arch/IBMQX3.def"

#undef EFD_ARCHITECTURE
#undef EFD_REG
#undef EFD_COUPLING
#undef EFD_ARCHITECTURE_END

template <> std::vector<std::string> efd::EnumArchitecture::mStrVal {
#define EFD_FIRSTLAST(_First_, _Last_)
#define EFD_ARCH(_Arch_, _Name_) \
    "A_"#_Name_,
#include "enfield/Arch/Architectures.def"
#undef EFD_FIRSTLAST
#undef EFD_ARCH
};

// Creating functions for each architecture, so that it
// will get the following signature: (int) -> ArchGraph*
#define EFD_FIRSTLAST(_First_, _Last_)
#define EFD_ARCH(_Arch_, _Name_) \
    ArchRegistry::RetTy CreateArch##_Arch_(int pad) {\
        auto arch = Arch##_Arch_::Create();\
        return ArchRegistry::RetTy(arch.release());\
    }
#include "enfield/Arch/Architectures.def"
#undef EFD_FIRSTLAST
#undef EFD_ARCH
}

static efd::ArchRegistryPtr GetRegistry() {
    static efd::ArchRegistryPtr Registry(new efd::ArchRegistry());
    return Registry;
}

void efd::InitializeAllArchitectures() {
#define EFD_FIRSTLAST(_First_, _Last_)
#define EFD_ARCH(_Arch_, _Name_) \
    RegisterArchitecture(Architecture::A_##_Name_, CreateArch##_Arch_);
#include "enfield/Arch/Architectures.def"
#undef EFD_FIRSTLAST
#undef EFD_ARCH
}

bool efd::HasArchitecture(EnumArchitecture key) {
    return GetRegistry()->hasObj(key);
}

void efd::RegisterArchitecture(EnumArchitecture key, ArchRegistry::CtorTy ctor) {
    GetRegistry()->registerObj(key, ctor);
}

efd::ArchRegistry::RetTy efd::CreateArchitecture(EnumArchitecture key) {
    return GetRegistry()->createObj(key, 0);
}
