
#include <iostream>

#include "enfield/Support/CommandLine.h"

efd::Opt<int> intOpt("int", "An int option.", 0);
efd::Opt<unsigned> ulOpt("ul", "An unsigned option.", 0);
efd::Opt<long long> llOpt("ll", "A long long option.", 0);
efd::Opt<unsigned long long> ullOpt("ull", "An unsigned long long option.", 0);
efd::Opt<std::string> strOpt("str", "A string option.", "");
efd::Opt<bool> boolOpt("bool", "A bool option.");

int main(int argc, char **argv) {
    efd::ParseArguments(argc, argv);
    std::cout << "Hello" << std::endl;
    return 0;
}
