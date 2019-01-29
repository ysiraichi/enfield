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

    /// \brief Two \p Swap objects are considered equal if they occur
    /// on equal qubits (order is irrelevant).
    inline bool operator==(const Swap& lhs, const Swap& rhs) {
        return (lhs.u == rhs.u && lhs.v == rhs.v) ||
               (lhs.u == rhs.v && lhs.v == rhs.u);
    }

    inline bool operator!=(const Swap& lhs, const Swap& rhs) {
        return !(lhs == rhs);
    }

    typedef std::vector<Swap> SwapSeq;

    /// \brief Returns a stream object for logging errors.
    std::ostream& ErrorLog(const std::string& file = "", const uint32_t& line = 0);
    /// \brief Returns a stream object for logging warnings.
    std::ostream& WarningLog(const std::string& file = "", const uint32_t& line = 0);
    /// \brief Returns a stream object for logging information.
    std::ostream& InfoLog(const std::string& file = "", const uint32_t& line = 0);

    /// \brief Initialize the log files.
    void InitializeLogs();

    /// \brief Aborts reporting the file and the line.
    void Abort [[noreturn]] (const std::string& file = "", const uint32_t& line = 0);
}

// Beginning of EFD_MESSAGE_LOG ifdef
#ifndef EFD_MESSAGE_LOG
#define EFD_MESSAGE_LOG

#define ERR efd::ErrorLog(__FILE__, __LINE__)
#define WAR efd::WarningLog(__FILE__, __LINE__)
#define INF efd::InfoLog(__FILE__, __LINE__)

#define EfdAbortIf(_Cond_, _Message_)   \
    if (_Cond_) {                       \
        ERR << _Message_ << std::endl;  \
        efd::Abort(__FILE__, __LINE__); \
    }

#endif
// End of EFD_MESSAGE_LOG ifdef

#endif
