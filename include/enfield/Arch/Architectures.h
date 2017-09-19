#ifndef __EFD_ARCHITECTURES_H__
#define __EFD_ARCHITECTURES_H__

#include "enfield/Arch/ArchGraph.h"
#include "enfield/Support/Registry.h"
namespace efd {
    typedef Registry<ArchGraph::uRef, int> ArchRegistry;

    void InitializeAllArchitectures();
    /// \brief Returns true if there is an architecture mapped by \p key;
    bool HasArchitecture(std::string key);
    /// \brief Register an architecture, mapping \p key to \p ctor.
    void RegisterArchitecture(std::string key, ArchRegistry::CtorTy ctor);
    /// \brief Creates an architecture referenced by \p name.
    ArchRegistry::RetTy CreateArchitecture(std::string key);
}

#endif
