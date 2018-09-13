#include "enfield/Arch/Architectures.h"
#include "enfield/Analysis/Nodes.h"
#include "enfield/Support/JsonParser.h"
#include "enfield/Support/RTTI.h"

namespace efd {
    typedef std::shared_ptr<ArchRegistry> ArchRegistryPtr;

    template <> std::vector<std::string> efd::EnumArchitecture::Self::mStrVal {
        "first",
#define EFD_ARCH(_Name_, _Json_) \
        "A_"#_Name_,
#include "enfield/Arch/Architectures.def"
#undef EFD_ARCH
        "last"
    };

// Creating functions for each architecture, so that it
// will get the following signature: (int) -> ArchGraph*
#define EFD_ARCH(_Name_, _Json_) \
    ArchRegistry::RetTy CreateArch_##_Name_(int pad) {\
        auto json = std::string(#_Json_);\
        auto arch = JsonParser<ArchGraph>::ParseString(json.substr(1, json.length() - 2));\
        return ArchRegistry::RetTy(arch.release());\
    }
#include "enfield/Arch/Architectures.def"
#undef EFD_ARCH
}

static efd::ArchRegistryPtr GetRegistry() {
    static efd::ArchRegistryPtr Registry(new efd::ArchRegistry());
    return Registry;
}

void efd::InitializeAllArchitectures() {
#define EFD_ARCH(_Name_, _Json_) \
    RegisterArchitecture(Architecture::A_##_Name_, CreateArch_##_Name_);
#include "enfield/Arch/Architectures.def"
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
