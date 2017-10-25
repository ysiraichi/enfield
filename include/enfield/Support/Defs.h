#ifndef __EFD_DEFS_H__
#define __EFD_DEFS_H__

#include <vector>
#include <cstdint>
#include <limits>

namespace efd {
    /// \brief Defines the type used for mapping the qubits.
    typedef std::vector<uint32_t> Mapping;
    typedef std::vector<uint32_t> Assign;

    /// \brief Constant should be used as an undefined in a mapping.
    static const uint32_t _undef = std::numeric_limits<uint32_t>::max();

    /// \brief Struct used for representing a swap between two qubits;
    struct Swap {
        uint32_t u;
        uint32_t v;
    };

    typedef std::vector<Swap> SwapSeq;
}

#endif
