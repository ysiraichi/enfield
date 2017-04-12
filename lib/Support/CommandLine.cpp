
#include "enfield/Support/CommandLine.h"

#include <unordered_map>
#include <memory>
#include <cassert>

using namespace efd;

namespace efd {

    class ArgsParser {
        public:
            std::unordered_map<std::string, OptBase*> mArgMap;

            ArgsParser();

            bool hasOpt(OptBase* opt);
            void addOpt(OptBase* opt);
    };

};

ArgsParser::ArgsParser() {}

bool ArgsParser::hasOpt(OptBase* opt) {
    return mArgMap.find(opt->mName) != mArgMap.end();
}

void ArgsParser::addOpt(OptBase* opt) {
    mArgMap.insert(std::pair<std::string, OptBase*>(opt->mName, opt));
}

static std::shared_ptr<ArgsParser> Parser(nullptr);

static std::shared_ptr<ArgsParser> getParser() {
    if (Parser.get() == nullptr)
        Parser.reset(new ArgsParser());

    return Parser;
}

OptBase::OptBase(std::string name, std::string description) {
    std::shared_ptr<ArgsParser> Parser = getParser();

    if (!Parser->hasOpt(this))
        Parser->addOpt(this);
}

void ParseArguments(int argc, char **argv) {
    std::shared_ptr<ArgsParser> Parser = getParser();

    for (int i = 1; i < argc; ++i) {
        std::string arg(argv[i]);

        if (Parser->mArgMap.find(arg) != Parser->mArgMap.end()) {
        }
    }

    for (auto pair : Parser->mArgMap) {
    }
}
