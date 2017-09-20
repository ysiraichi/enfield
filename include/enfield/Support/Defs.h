#ifndef __EFD_DEFS_H__
#define __EFD_DEFS_H__

#include <vector>

namespace efd {
    /// \brief Defines the type used for mapping the qubits.
    typedef std::vector<unsigned> Mapping;
    typedef std::vector<unsigned> Assign;

    /// \brief Struct used for representing a swap between two qubits;
    struct Swap {
        unsigned u;
        unsigned v;
    };
}

#endif
