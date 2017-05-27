#include "enfield/Support/CommandLine.h"

#include <unordered_map>

static std::string Qelib1 =
#define EFD_LIB(...) #__VA_ARGS__
#include "enfield/StdLib/qelib1.inc"
#undef EFD_LIB
;

efd::Opt<std::vector<std::string>> IncludePath
    ("I", "The include path.", std::vector<std::string>(), false);

std::unordered_map<std::string, std::string> StdLib = {
    { "qelib1.inc", Qelib1 }
};
