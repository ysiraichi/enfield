#ifndef __EFD_DEFS_H__
#define __EFD_DEFS_H__

#include <vector>
#include <cstdint>
#include <limits>
#include <ostream>

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

    /// \brief Returns a stream object for logging errors.
    std::ostream& ErrorLog(std::string file = "", uint32_t line = 0);
    /// \brief Returns a stream object for logging warnings.
    std::ostream& WarningLog(std::string file = "", uint32_t line = 0);
    /// \brief Returns a stream object for logging information.
    std::ostream& InfoLog(std::string file = "", uint32_t line = 0);

    /// \brief Instead of issueing an 'assert(false)', we should use
    /// this enum in order to exit with errors.
    enum class ExitCode {
        EXIT_multi_deps = 11,
        EXIT_unreachable
    };
}

#ifndef EFD_MESSAGE_LOG
#define EFD_MESSAGE_LOG
#define ERR efd::ErrorLog(__FILE__, __LINE__)
#define WAR efd::WarningLog(__FILE__, __LINE__)
#define INF efd::InfoLog(__FILE__, __LINE__)
#endif

#endif
