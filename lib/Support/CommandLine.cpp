
#include "enfield/Support/CommandLine.h"

#include <cstdlib>

#include <iostream>
#include <iomanip>

#include <unordered_map>
#include <vector>
#include <memory>

using namespace efd;

namespace efd {

    class ArgsParser {
        public:
            std::unordered_map<std::string, std::vector<OptBase*> > mArgMap;

            ArgsParser() {}

            bool hasOpt(OptBase* opt);
            void addOpt(OptBase* opt);
    };

};

bool ArgsParser::hasOpt(OptBase* opt) {
    return mArgMap.find(opt->mName) != mArgMap.end();
}

void ArgsParser::addOpt(OptBase* opt) {
    if (!hasOpt(opt)) {
        std::string name(opt->mName);
        mArgMap.insert(std::make_pair(name, std::vector<OptBase*>()));
    }

    mArgMap[opt->mName].push_back(opt);
}

static std::shared_ptr<ArgsParser> Parser(nullptr);
static efd::Opt<bool> PrintHelp("help", "Prints a list with the available commands.", false, false);

static std::shared_ptr<ArgsParser> getParser() {
    if (Parser.get() == nullptr)
        Parser.reset(new ArgsParser());

    return Parser;
}

template <>
void efd::Opt<bool>::parse(int argc, char **argv) {
    mVal = true;
}

template <>
void efd::Opt<int>::parse(int argc, char **argv) {
    assert(argc > 2 && "Error parsing the arguments.");
    mVal = std::stoi(std::string(argv[1]));
}

template <>
void efd::Opt<unsigned>::parse(int argc, char **argv) {
    assert(argc > 2 && "Error parsing the arguments.");
    mVal = std::stoul(std::string(argv[1]));
}

template <>
void efd::Opt<long long>::parse(int argc, char **argv) {
    assert(argc > 2 && "Error parsing the arguments.");
    mVal = std::stoll(std::string(argv[1]));
}

template <>
void efd::Opt<unsigned long long>::parse(int argc, char **argv) {
    assert(argc > 2 && "Error parsing the arguments.");
    mVal = std::stoull(std::string(argv[1]));
}

template <>
void efd::Opt<float>::parse(int argc, char **argv) {
    assert(argc > 2 && "Error parsing the arguments.");
    mVal = std::stof(std::string(argv[1]));
}

template <>
void efd::Opt<double>::parse(int argc, char **argv) {
    assert(argc > 2 && "Error parsing the arguments.");
    mVal = std::stod(std::string(argv[1]));
}

template <>
void efd::Opt<std::string>::parse(int argc, char **argv) {
    assert(argc > 2 && "Error parsing the arguments.");
    mVal = std::string(argv[1]);
}

efd::OptBase::OptBase(std::string name, std::string description, bool isRequired) : 
    mName(name), mDescription(description), mIsRequired(isRequired), mIsParsed(false) {

    std::shared_ptr<ArgsParser> Parser = getParser();
    Parser->addOpt(this);
}

bool efd::OptBase::isParsed() {
    return mIsParsed;
}

bool efd::OptBase::isRequired() {
    return mIsRequired;
}

static void PrintCommandLineHelp() {
    std::shared_ptr<ArgsParser> Parser = getParser();

    std::cout << "Arguments:" << std::endl;

    const int nCols = 20;
    for (auto pair : Parser->mArgMap) {
        std::cout << "\t" << std::left << std::setw(nCols) << "-" + pair.first;
        std::cout << pair.second[0]->mDescription << std::endl;
    }
}

void efd::ParseArguments(int argc, char **argv) {
    std::shared_ptr<ArgsParser> Parser = getParser();

    for (int i = 1; i < argc; ++i) {
        // Removing the initial '-'
        char *argName = argv[i] + 1;
        std::string arg(argName);

        char **argvI = argv + i;
        int argcI = argc - i;

        if (Parser->mArgMap.find(arg) != Parser->mArgMap.end()) {
            for (OptBase *opt : Parser->mArgMap[arg])
                opt->parse(argcI, argvI);
        }
    }

    bool requirementsFulfilled = true;
    for (auto pair : Parser->mArgMap) {
        OptBase *opt = pair.second[0];
        if (opt->isRequired() && !opt->isParsed()) {
            requirementsFulfilled = false;
            break;
        }
    }

    if (PrintHelp.getVal() || !requirementsFulfilled) {
        PrintCommandLineHelp();
        abort();
    }
}
