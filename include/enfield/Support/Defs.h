#ifndef __EFD_DEFS_H__
#define __EFD_DEFS_H__

#include <vector>
#include <cstdint>
#include <limits>
#include <ostream>

namespace efd {
    /// \brief Defines the type used for mapping the qubits.
    typedef std::vector<uint32_t> Mapping;
    typedef std::vector<uint32_t> InverseMap;

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

    /// \brief Aborts reporting the file and the line.
    void Abort(std::string file = "", uint32_t line = 0);
}

#ifndef EFD_MESSAGE_LOG
#define EFD_MESSAGE_LOG
#define EFD_ABORT() Abort(__FILE__, __LINE__)
#define ERR ErrorLog(__FILE__, __LINE__)
#define WAR WarningLog(__FILE__, __LINE__)
#define INF InfoLog(__FILE__, __LINE__)
#endif

#endif
