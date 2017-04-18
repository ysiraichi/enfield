
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
        for (std::vector<OptBase*>::iterator it = optVector.begin(), e = optVector.end(); it != e; ++it)
            if (opt == *it) optVector.erase(it);

        if (optVector.empty())
            mArgMap.erase(mArgMap.find(opt->mName));
    }
}

static std::shared_ptr<ArgsParser> Parser(nullptr);
static efd::Opt<bool> PrintHelp("help", "Prints a list with the available commands.", false, false);

static std::shared_ptr<ArgsParser> getParser() {
    if (Parser.get() == nullptr)
        Parser.reset(new ArgsParser());

    return Parser;
}


template <>
void efd::Opt<bool>::parse(const int argc, const char **argv) {
    mVal = true;
}

template <>
bool efd::Opt<bool>::isBoolean() { return true; }

template <>
void efd::Opt<int>::parse(const int argc, const char **argv) {
    assert(argc >= 2 && "Not enough command line arguments.");
    mVal = std::stoi(std::string(argv[1]));
}

template <>
void efd::Opt<unsigned>::parse(const int argc, const char **argv) {
    assert(argc >= 2 && "Not enough command line arguments.");
    mVal = std::stoul(std::string(argv[1]));
}

template <>
void efd::Opt<long long>::parse(const int argc, const char **argv) {
    assert(argc >= 2 && "Not enough command line arguments.");
    mVal = std::stoll(std::string(argv[1]));
}

template <>
void efd::Opt<unsigned long long>::parse(const int argc, const char **argv) {
    assert(argc >= 2 && "Not enough command line arguments.");
    mVal = std::stoull(std::string(argv[1]));
}

template <>
void efd::Opt<float>::parse(const int argc, const char **argv) {
    assert(argc >= 2 && "Not enough command line arguments.");
    mVal = std::stof(std::string(argv[1]));
}

template <>
void efd::Opt<double>::parse(const int argc, const char **argv) {
    assert(argc >= 2 && "Not enough command line arguments.");
    mVal = std::stod(std::string(argv[1]));
}

template <>
void efd::Opt<std::string>::parse(const int argc, const char **argv) {
    assert(argc >= 2 && "Not enough command line arguments.");
    mVal = std::string(argv[1]);
}

efd::OptBase::OptBase(std::string name, std::string description, bool isRequired) : 
    mName(name), mDescription(description), mIsRequired(isRequired), mIsParsed(false) {

    std::shared_ptr<ArgsParser> Parser = getParser();
    Parser->addOpt(this);
}

efd::OptBase::~OptBase() {
    std::shared_ptr<ArgsParser> Parser = getParser();
    Parser->delOpt(this);
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

void efd::ParseArguments(const int argc, const char **argv) {
    std::shared_ptr<ArgsParser> Parser = getParser();

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
        assert(false && "ArgsParser not able to parse arguments correctly.");
    }
}
