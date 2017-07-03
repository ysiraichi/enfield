#include "enfield/Support/CommandLine.h"
#include "enfield/Transform/QModule.h"
#include "enfield/Transform/FlattenPass.h"
#include "enfield/Transform/DependencyBuilderPass.h"
#include "enfield/Transform/WeightedPMQbitAllocator.h"
#include "enfield/Transform/DynProgQbitAllocator.h"
#include "enfield/Transform/ReverseEdgesPass.h"
#include "enfield/Support/OneRestrictionSwapFinder.h"
#include "enfield/Arch/Architectures.h"
#include "enfield/Support/Stats.h"

#include <fstream>
#include <cassert>

using namespace efd;

static Opt<std::string> InFilepath
("i", "The input file.", "/dev/stdin", true);
static Opt<std::string> OutFilepath
("o", "The output file.", "/dev/stdout", false);

static Opt<bool> Pretty
("pretty", "Print in a pretty format.", true, false);
static Opt<bool> ShowStats
("stats", "Print statistical data collected.", false, false);

// TODO: This should be change to a nicer interface.
static Opt<std::string> Allocator
("alloc", "Sets the allocator to be used. \
Default: DynProg. \
Options: DynProg; WPM.", "DynProg", false);

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

        qmod->runPass(flattenPass);
        qmod->runPass(qbitUidPass);

        // Architecture-dependent fragment.
        std::unique_ptr<ArchIBMQX2> graph = ArchIBMQX2::Create();
        assert(qbitUidPass->getSize() <= graph->size() &&
                "Using more qbits than the maximum.");

        QbitAllocator* allocator;
        if (Allocator.getVal() == "WPM")
            allocator = WeightedPMQbitAllocator::Create(qmod.get(), graph.get());
        else allocator = DynProgQbitAllocator::Create(qmod.get(), graph.get());
        allocator->setInlineAll({ "cx", "u1", "u2", "u3" });
        allocator->run();

        // Reversing the edges.
        ReverseEdgesPass* revPass = ReverseEdgesPass::Create(qmod.get(), graph.get());
        qmod->runPass(revPass);

        DumpToOutFile(qmod.get());
    }

    if (ShowStats.getVal())
        efd::PrintStats();

    return 0;
}
