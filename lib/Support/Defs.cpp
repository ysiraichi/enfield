#include "enfield/Support/Defs.h"
#include "enfield/Support/CommandLine.h"

#include <iostream>
#include <fstream>

#define EFD_PREFIX_COLOR "\e[38;5;"
#define EFD_RESET_COLOR "\e[0m"

#ifndef EFD_ERR_COLOR
#define EFD_ERR_COLOR 9
#endif

#ifndef EFD_WAR_COLOR
#define EFD_WAR_COLOR 3
#endif

#ifndef EFD_INF_COLOR
#define EFD_INF_COLOR 14
#endif

static efd::Opt<std::string> ErrorFile
("err", "File to keep error messages.", "", false);
static efd::Opt<std::string> WarningFile
("war", "File to keep warning messages.", "", false);
static efd::Opt<std::string> InfoFile
("inf", "File to keep information messages.", "", false);

static efd::Opt<bool> NoColor
("-no-color", "Do not print color characters.", false, false);
static efd::Opt<bool> Verbose
("v", "Verbose. You know... That thing everyone does!", false, false);

static inline
void PrintMessage(std::ostream& out,
                  const std::string& prefix,
                  const std::string& file,
                  const uint32_t& line) {
    out << prefix;

    if (Verbose.getVal() && file != "" && line != 0) {
        out << file << ":" << line << ":" << std::endl;
        out << "\t";
    }
}

#define EFD_IMPLEMENT_LOG(_fnName_, _level_, _cl_, _default_, _color_)              \
    static std::ostream* _fnName_##Stream = &_default_;                             \
    static const std::string _fnName_##UncoloredPrefix = "[" _level_ "]: ";         \
    static const std::string _fnName_##ColoredPrefix =                              \
        EFD_PREFIX_COLOR #_color_ "m" + _fnName_##UncoloredPrefix + EFD_RESET_COLOR;\
    static const std::string* _fnName_##Prefix = &_fnName_##ColoredPrefix;          \
                                                                                    \
    std::ostream& efd::_fnName_(const std::string& file, const uint32_t& line) {    \
        PrintMessage(*_fnName_##Stream, *_fnName_##Prefix, file, line);             \
        return *_fnName_##Stream;                                                   \
    }                                                                               \
                                                                                    \
    static void Initialize##_fnName_() {                                            \
        static std::ofstream _fnName_##ClFile;                                      \
        if (!_cl_.getVal().empty()) {                                               \
            _fnName_##ClFile.open(_cl_.getVal());                                   \
            _fnName_##Stream = &_fnName_##ClFile;                                   \
            _fnName_##Prefix = &_fnName_##UncoloredPrefix;                          \
        } else if (NoColor.getVal()) {                                              \
            _fnName_##Prefix = &_fnName_##UncoloredPrefix;                          \
        }                                                                           \
    }

EFD_IMPLEMENT_LOG(ErrorLog, "ERROR", ErrorFile, std::cerr, 9)
EFD_IMPLEMENT_LOG(WarningLog, "WARNING", WarningFile, std::cout, 3)
EFD_IMPLEMENT_LOG(InfoLog, "INFO", InfoFile, std::cout, 2)

void efd::InitializeLogs() {
    InitializeErrorLog();
    InitializeWarningLog();
    InitializeInfoLog();
}

void efd::Abort(const std::string& file, const uint32_t& line) {
    std::cerr << "Enfield Aborted on file `" << file << "`, line `" << line << "`." 
              << std::endl;
    std::abort();
}
