
#include "enfield/Support/CommandLine.h"
#include "enfield/Support/Defs.h"

#include <cstdlib>

#include <iostream>
#include <iomanip>

#include <map>
#include <vector>
#include <memory>

using namespace efd;

namespace efd {

    class ArgsParser {
        public:
            std::map<std::string, std::vector<OptBase*> > mArgMap;

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
    std::string name(opt->mName);

    if (!hasOpt(opt)) {
        mArgMap.insert(std::make_pair(name, std::vector<OptBase*>()));
    }

    mArgMap[name].push_back(opt);
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

void efd::ParseOptTrait<bool>::
Run(Opt<bool>* opt, std::vector<std::string> args) {
    opt->mVal = !opt->mVal;
}

void efd::ParseOptTrait<int>::
Run(Opt<int>* opt, std::vector<std::string> args) {
    opt->mVal = std::stoi(args[0]);
}

void efd::ParseOptTrait<uint32_t>::
Run(Opt<uint32_t>* opt, std::vector<std::string> args) {
    opt->mVal = std::stoul(args[0]);
}

void efd::ParseOptTrait<long long>::
Run(Opt<long long>* opt, std::vector<std::string> args) {
    opt->mVal = std::stoll(args[0]);
}

void efd::ParseOptTrait<unsigned long long>::
Run(Opt<unsigned long long>* opt, std::vector<std::string> args) {
    opt->mVal = std::stoull(args[0]);
}

void efd::ParseOptTrait<float>::
Run(Opt<float>* opt, std::vector<std::string> args) {
    opt->mVal = std::stof(args[0]);
}

void efd::ParseOptTrait<double>::
Run(Opt<double>* opt, std::vector<std::string> args) {
    opt->mVal = std::stod(args[0]);
}

void efd::ParseOptTrait<std::string>::
Run(Opt<std::string>* opt, std::vector<std::string> args) {
    opt->mVal = args[0];
}

void efd::ParseOptTrait<std::vector<std::string>>::
Run(Opt<std::vector<std::string>>* opt, std::vector<std::string> args) {
    opt->mVal.push_back(args[0]);
}

template <>
uint32_t efd::Opt<bool>::argsConsumed() {
    return 0;
}

template <>
std::string efd::Opt<bool>::getStringVal() {
    if (mVal) return "true";
    else return "false";
}

template <>
std::string efd::Opt<std::string>::getStringVal() {
    return mVal;
}

template <>
std::string efd::Opt<std::vector<std::string>>::getStringVal() {
    std::string str;
    for (auto s : mVal) str += s + "; ";
    return str;
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

void efd::OptBase::parse(std::vector<std::string> args) {
    parseImpl(args);
    mIsParsed = true;
}

static void PrintOption(OptBase* ptr, bool printDefault = true) {
    static const int nCols = 20;
    std::cout << "\t" << std::left << std::setw(nCols) << "-" + ptr->mName;
    std::cout << ptr->mDescription << std::endl;

    auto possibleValues = ptr->getPossibleValuesList();
    if (!possibleValues.empty()) {
        std::cout << "\t" << std::left << std::setw(nCols) << " " << "Possible: [";

        for (uint32_t i = 0, e = possibleValues.size(); i < e; ++i) {
            std::cout << possibleValues[i];
            if (i < e - 1) std::cout << ", ";
        }

        std::cout << "]" << std::endl;
    }

    if (printDefault) {
        std::cout << "\t" << std::left << std::setw(nCols) << " " <<
            "Default: " << ptr->getStringVal() << std::endl;
    }
}

static void PrintCommandLineHelp() {
    std::shared_ptr<ArgsParser> Parser = GetParser();

    std::cout << "Usage:" << std::endl;
    std::cout << "\t$ efd <required> [Options]" << std::endl;

    std::cout << std::endl;
    std::cout << "Examples:" << std::endl;
    std::cout << "\tCompiles the file 'test.qasm' using 'Q_dynprog' and 'A_ibmqx2' ";
    std::cout << "(default values)" << std::endl;
    std::cout << "\t$ efd -i test.qasm" << std::endl;
    std::cout << std::endl;

    std::cout << "\tCompiles the file 'test.qasm' using 'Q_wpm' and 'A_ibmqx3'." << std::endl;
    std::cout << "\tIn the end, it shows some statistical data." << std::endl;
    std::cout << "\t$ efd -i test.qasm -alloc Q_wpm -arch A_ibmqx3 -stats" << std::endl;
    std::cout << std::endl;

    std::cout << "\tCompiles the file 'test.qasm' using 'Q_wpm' and a custom" << std::endl;
    std::cout << "\tarchitecture specified in 'folder/arch'. In the end, it shows some " << std::endl;
    std::cout << "\tstatistical data." << std::endl;
    std::cout << "\t$ efd -i test.qasm -alloc Q_wpm -arch-file folder/arch -stats" << std::endl;

    std::cout << std::endl;
    std::cout << "Required:" << std::endl;

    for (auto pair : Parser->mArgMap) {
        if (pair.second[0]->isRequired()) {
            PrintOption(pair.second[0], false);
        }
    }

    std::cout << std::endl;
    std::cout << "Options:" << std::endl;

    for (auto pair : Parser->mArgMap) {
        if (!pair.second[0]->isRequired()) {
            PrintOption(pair.second[0]);
        }
    }
}

void efd::ParseArguments(int argc, char** argv) {
    const int cArgc = argc;
    const char** cArgv = (const char**) argv;
    efd::ParseArguments(cArgc, cArgv);
}

void efd::ParseArguments(const int argc, const char **argv) {
    std::shared_ptr<ArgsParser> Parser = GetParser();
    std::vector<std::string> rawArgs;

    for (int32_t i = 0; i < argc; ++i)
        rawArgs.push_back(argv[i]);

    for (int32_t i = 1; i < argc; ++i) {
        std::string arg(rawArgs[i]);

        // Removing the initial '-'
        if (arg[0] == '-') {
            arg = arg.substr(1);
        }

        if (Parser->mArgMap.find(arg) != Parser->mArgMap.end()) {
            std::vector<OptBase*>& optVector = Parser->mArgMap[arg];

            int32_t toBeConsumed = optVector[0]->argsConsumed();

            EfdAbortIf(i + toBeConsumed >= argc,
                       "There should be " << toBeConsumed << " arguments for -"
                       << arg << ", but there was only " << argc - i << ".");

            std::vector<std::string> optArgs;
            for (int32_t j = 0; j < toBeConsumed; ++j) {
                optArgs.push_back(rawArgs[i + 1 + j]);
            }

            for (OptBase *opt : optVector) {
                opt->parse(optArgs);
            }

            i += toBeConsumed;
        } else {
            WAR << "CommandLine argument '" << arg << "' not used!" << std::endl;
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
        std::exit(0);
    }
}
