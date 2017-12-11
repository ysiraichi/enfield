
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
            void delOpt(OptBase* opt);
    };

};

bool efd::ArgsParser::hasOpt(OptBase* opt) {
    return mArgMap.find(opt->mName) != mArgMap.end();
}

void efd::ArgsParser::addOpt(OptBase* opt) {
    if (!hasOpt(opt)) {
        std::string name(opt->mName);
        mArgMap.insert(std::make_pair(name, std::vector<OptBase*>()));
    }

    mArgMap[opt->mName].push_back(opt);
}

void efd::ArgsParser::delOpt(OptBase* opt) {
    if (hasOpt(opt)) {
        std::vector<OptBase*> &optVector = mArgMap[opt->mName];

        if (optVector.size() == 1)
            mArgMap.erase(mArgMap.find(opt->mName));
        else {
            for (auto it = optVector.begin(), e = optVector.end(); it != e; ++it)
                if (opt == *it) {
                    optVector.erase(it);
                    break;
                }
        }

    }
}

static efd::Opt<bool> PrintHelp
("help", "Prints a list with the available commands.", false, false);

static std::shared_ptr<ArgsParser> GetParser() {
    static std::shared_ptr<ArgsParser> Parser(new ArgsParser());
    return Parser;
}

template <>
void efd::Opt<bool>::parseImpl(const int argc, const char **argv) {
    mVal = !mVal;
}

template <>
bool efd::Opt<bool>::isBoolean() { return true; }

template <>
void efd::Opt<int>::parseImpl(const int argc, const char **argv) {
    assert(argc >= 2 && "Not enough command line arguments.");
    mVal = std::stoi(std::string(argv[1]));
}

template <>
void efd::Opt<uint32_t>::parseImpl(const int argc, const char **argv) {
    assert(argc >= 2 && "Not enough command line arguments.");
    mVal = std::stoul(std::string(argv[1]));
}

template <>
void efd::Opt<long long>::parseImpl(const int argc, const char **argv) {
    assert(argc >= 2 && "Not enough command line arguments.");
    mVal = std::stoll(std::string(argv[1]));
}

template <>
void efd::Opt<unsigned long long>::parseImpl(const int argc, const char **argv) {
    assert(argc >= 2 && "Not enough command line arguments.");
    mVal = std::stoull(std::string(argv[1]));
}

template <>
void efd::Opt<float>::parseImpl(const int argc, const char **argv) {
    assert(argc >= 2 && "Not enough command line arguments.");
    mVal = std::stof(std::string(argv[1]));
}

template <>
void efd::Opt<double>::parseImpl(const int argc, const char **argv) {
    assert(argc >= 2 && "Not enough command line arguments.");
    mVal = std::stod(std::string(argv[1]));
}

template <>
void efd::Opt<std::string>::parseImpl(const int argc, const char **argv) {
    assert(argc >= 2 && "Not enough command line arguments.");
    mVal = std::string(argv[1]);
}

template <>
void efd::Opt<std::vector<std::string>>::parseImpl(const int argc, const char **argv) {
    assert(argc >= 2 && "Not enough command line arguments.");
    mVal.push_back(std::string(argv[1]));
}

efd::OptBase::OptBase(std::string name, std::string description, bool isRequired) : 
    mIsRequired(isRequired), mIsParsed(false), mName(name), mDescription(description) {

    mParser = GetParser();
    mParser->addOpt(this);
}

efd::OptBase::~OptBase() {
    mParser->delOpt(this);
}

bool efd::OptBase::isParsed() {
    return mIsParsed;
}

bool efd::OptBase::isRequired() {
    return mIsRequired;
}

void efd::OptBase::parse(const int argc, const char **argv) {
    parseImpl(argc, argv);
    mIsParsed = true;
}

static void PrintCommandLineHelp() {
    std::shared_ptr<ArgsParser> Parser = GetParser();

    std::cout << "Arguments:" << std::endl;

    const int nCols = 20;
    for (auto pair : Parser->mArgMap) {
        std::cout << "\t" << std::left << std::setw(nCols) << "-" + pair.first;
        std::cout << pair.second[0]->mDescription << std::endl;
    }
}

void efd::ParseArguments(int argc, char** argv) {
    const int cArgc = argc;
    const char** cArgv = (const char**) argv;
    efd::ParseArguments(cArgc, cArgv);
}

void efd::ParseArguments(const int argc, const char **argv) {
    std::shared_ptr<ArgsParser> Parser = GetParser();

    for (int i = 1; i < argc; ++i) {
        // Removing the initial '-'
        const char *argName = argv[i] + 1;
        std::string arg(argName);

        const char **argvI = argv + i;
        const int argcI = argc - i;

        if (Parser->mArgMap.find(arg) != Parser->mArgMap.end()) {
            std::vector<OptBase*>& optVector = Parser->mArgMap[arg];

            for (OptBase *opt : optVector)
                opt->parse(argcI, argvI);

            if (!optVector[0]->isBoolean()) ++i;
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
        exit(0);
    }
}
