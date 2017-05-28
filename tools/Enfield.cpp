#include "enfield/Support/CommandLine.h"
#include "enfield/Transform/QModule.h"
#include "enfield/Transform/FlattenPass.h"
#include "enfield/Transform/DependencyBuilderPass.h"
#include "enfield/Transform/DynProgQbitAllocator.h"
#include "enfield/Transform/ReverseEdgesPass.h"
#include "enfield/Support/OneRestrictionSwapFinder.h"
#include "enfield/Arch/Architectures.h"

#include <fstream>
#include <cassert>

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
        // Creating default passes.
        FlattenPass* flattenPass = FlattenPass::Create(qmod.get());
        QbitToNumberPass* qbitUidPass = QbitToNumberPass::Create();
        DependencyBuilderPass* depPass = DependencyBuilderPass::Create
            (qmod.get(), qbitUidPass);

        qmod->runPass(flattenPass);
        qmod->runPass(qbitUidPass);
        qmod->runPass(depPass);

        // Architecture-dependent fragment.
        std::unique_ptr<ArchIBMQX2> graph = ArchIBMQX2::Create();
        assert(qbitUidPass->getSize() <= graph->size() &&
                "Using more qbits than the maximum.");

        OneRestrictionSwapFinder* swapFinder = OneRestrictionSwapFinder::Create(graph.get());
        DynProgQbitAllocator* dynAllocator = DynProgQbitAllocator::Create
            (qmod.get(), graph.get(), swapFinder, depPass);
        dynAllocator->run();

        // Reversing the edges.
        ReverseEdgesPass* revPass = ReverseEdgesPass::Create(qmod.get(), graph.get(), depPass);
        qmod->runPass(revPass);

        DumpToOutFile(qmod.get());
    }

    return 0;
}
