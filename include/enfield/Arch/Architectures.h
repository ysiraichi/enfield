#ifndef __EFD_ARCHITECTURES_H__
#define __EFD_ARCHITECTURES_H__

#include "enfield/Arch/ArchGraph.h"
#include "enfield/Support/Registry.h"
#include "enfield/Support/EnumString.h"

namespace efd {
    enum class Architecture {
#define EFD_FIRSTLAST(_First_, _Last_)
#define EFD_ARCH(_Arch_, _Name_)\
        A_##_Name_,
#include "enfield/Arch/Architectures.def"
#undef EFD_FIRSTLAST
#undef EFD_ARCH
    };

#define EFD_FIRSTLAST(_First_, _Last_)\
    typedef EnumString<Architecture,\
                       Architecture::A_##_First_,\
                       Architecture::A_##_Last_>\
            EnumArchitecture;
#define EFD_ARCH(_Arch_, _Name_)
#include "enfield/Arch/Architectures.def"
#undef EFD_FIRSTLAST
#undef EFD_ARCH

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
