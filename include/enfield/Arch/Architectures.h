#ifndef __EFD_ARCHITECTURES_H__
#define __EFD_ARCHITECTURES_H__

#include "enfield/Arch/ArchGraph.h"
#include "enfield/Support/Registry.h"
#include "enfield/Support/EnumString.h"
#include "enfield/Support/PossibleValuesListTrait.h"

namespace efd {
    enum class Architecture {
        first,
#define EFD_ARCH(_Name_, _Json_)\
        A_##_Name_,
#include "enfield/Arch/Architectures.def"
#undef EFD_ARCH
        last
    };

    typedef EnumString<Architecture, Architecture::first, Architecture::last, 1> EnumArchitecture;
    template <> std::vector<std::string> EnumArchitecture::mStrVal;

    typedef Registry<ArchGraph::uRef, int, EnumArchitecture> ArchRegistry;

    /// \brief Registers all available architectures in the register.
    void InitializeAllArchitectures();
    /// \brief Returns true if there is an architecture mapped by \p key;
    bool HasArchitecture(EnumArchitecture key);
    /// \brief Register an architecture, mapping \p key to \p ctor.
    void RegisterArchitecture(EnumArchitecture key, ArchRegistry::CtorTy ctor);
    /// \brief Creates an architecture referenced by \p name.
    ArchRegistry::RetTy CreateArchitecture(EnumArchitecture key);
}

#endif
