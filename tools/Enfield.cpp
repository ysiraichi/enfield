#include "enfield/Support/CommandLine.h"
#include "enfield/Transform/QModule.h"
#include "enfield/Transform/FlattenPass.h"
#include "enfield/Transform/DependencyBuilderPass.h"
#include "enfield/Transform/DynProgQbitAllocator.h"
#include "enfield/Support/OneRestrictionSwapFinder.h"
#include "enfield/Arch/Architectures.h"

#include <fstream>

using namespace efd;

static Opt<std::string> InFilepath("i", "The input file.", "/dev/stdin", true);
static Opt<std::string> OutFilepath("o", "The output file.", "/dev/stdout", false);

static Opt<bool> Pretty("pretty", "Print in a pretty format.", true, false);

static void DumpToOutFile(QModule* qmod) {
    std::ofstream O(OutFilepath.getVal());
    qmod->print(O, Pretty.getVal());
    O.close();
}

int main(int argc, char** argv) {
    ParseArguments(argc, argv);

    std::unique_ptr<QModule> qmod = QModule::Parse(InFilepath.getVal());

    if (qmod.get() != nullptr) {
        FlattenPass* flattenPass = FlattenPass::Create(qmod.get());
        QbitToNumberPass* qbitUidPass = QbitToNumberPass::Create();
        DependencyBuilderPass* depPass = DependencyBuilderPass::Create
            (qmod.get(), qbitUidPass);

        qmod->runPass(flattenPass);
        qmod->runPass(qbitUidPass);
        qmod->runPass(depPass);

        std::unique_ptr<ArchIBMQX2> graph = ArchIBMQX2::Create();
        OneRestrictionSwapFinder* swapFinder = OneRestrictionSwapFinder::Create(graph.get());
        DynProgQbitAllocator* dynAllocator = DynProgQbitAllocator::Create
            (qmod.get(), graph.get(), swapFinder, depPass);

        dynAllocator->run();

        DumpToOutFile(qmod.get());
    }

    return 0;
}
