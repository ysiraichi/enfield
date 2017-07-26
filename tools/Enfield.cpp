#include "enfield/Support/CommandLine.h"
#include "enfield/Transform/QModule.h"
#include "enfield/Transform/FlattenPass.h"
#include "enfield/Transform/QbitToNumberPass.h"
#include "enfield/Transform/DependencyBuilderPass.h"
#include "enfield/Transform/WeightedPMQbitAllocator.h"
#include "enfield/Transform/DynProgQbitAllocator.h"
#include "enfield/Transform/ReverseEdgesPass.h"
#include "enfield/Support/OneRestrictionSwapFinder.h"
#include "enfield/Arch/Architectures.h"
#include "enfield/Support/Stats.h"
#include "enfield/Support/uRefCast.h"

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

    QModule::sRef qmod = toShared(QModule::Parse(InFilepath.getVal()));

    if (qmod.get() != nullptr) {
        // Creating default passes.
        auto flattenPass = FlattenPass::Create();
        auto qbitUidPass = QbitToNumberWrapperPass::Create();

        flattenPass->run(qmod.get());
        qbitUidPass->run(qmod.get());

        auto qbitToNumber = qbitUidPass->getData(); 
        // Architecture-dependent fragment.
        auto graph = toShared(ArchIBMQX2::Create());
        assert(qbitToNumber.getSize() <= graph->size() &&
                "Using more qbits than the maximum.");

        QbitAllocator::Ref allocator;
        if (Allocator.getVal() == "WPM")
            allocator = WeightedPMQbitAllocator::Create(graph).release();
        else allocator = DynProgQbitAllocator::Create(graph).release();
        allocator->setInlineAll({ "cx", "u1", "u2", "u3" });
        allocator->run(qmod.get());

        // Reversing the edges.
        auto revPass = ReverseEdgesPass::Create(graph);
        revPass->run(qmod.get());

        DumpToOutFile(qmod.get());

        delete allocator;
    }

    if (ShowStats.getVal())
        efd::PrintStats();

    return 0;
}
