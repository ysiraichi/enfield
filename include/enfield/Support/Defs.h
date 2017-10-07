#ifndef __EFD_DEFS_H__
#define __EFD_DEFS_H__

#include <vector>
#include <cstdint>

namespace efd {
    /// \brief Defines the type used for mapping the qubits.
    typedef std::vector<uint32_t> Mapping;
    typedef std::vector<uint32_t> Assign;

    /// \brief Struct used for representing a swap between two qubits;
    struct Swap {
        uint32_t u;
        uint32_t v;
    };

    typedef std::vector<Swap> SwapSeq;
}

#endif
