#include "enfield/Support/Defs.h"
#include "enfield/Support/CommandLine.h"

#include <fstream>

static efd::Opt<std::string> ErrorFile
("err", "File to keep error messages.", "/dev/stderr", false);
static efd::Opt<std::string> WarningFile
("war", "File to keep warning messages.", "/dev/stdout", false);
static efd::Opt<std::string> InfoFile
("inf", "File to keep information messages.", "/dev/stdout", false);

static efd::Opt<bool> VerboseInfo
("v", "Show location of messages.", false, false);

static void PrintMessage(std::ostream& out,
                         std::string level, std::string file,
                         uint32_t line) {
    out << "[" << level << "]: ";

    if (VerboseInfo.getVal() && file != "" && line != 0) {
        out << file << ":" << line << ":" << std::endl;
        out << "\t";
    }
}

std::ostream& efd::ErrorLog(std::string file, uint32_t line) {
    static std::ofstream out(ErrorFile.getVal());
    PrintMessage(out, "ERROR", file, line);
    return out;
}

std::ostream& efd::WarningLog(std::string file, uint32_t line) {
    static std::ofstream out(WarningFile.getVal());
    PrintMessage(out, "WARNING", file, line);
    return out;
}

std::ostream& efd::InfoLog(std::string file, uint32_t line) {
    static std::ofstream out(InfoFile.getVal());
    PrintMessage(out, "INFO", file, line);
    return out;
}

void efd::ExitWith(ExitCode e) {
    std::exit(static_cast<uint32_t>(e));
}
