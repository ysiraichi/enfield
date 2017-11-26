#include "enfield/Support/CommandLine.h"
#include "enfield/Transform/Allocators.h"
#include "enfield/Transform/QModule.h"
#include "enfield/Transform/FlattenPass.h"
#include "enfield/Transform/XbitToNumberPass.h"
#include "enfield/Transform/DependencyBuilderPass.h"
#include "enfield/Transform/DynProgQbitAllocator.h"
#include "enfield/Transform/ReverseEdgesPass.h"
#include "enfield/Transform/CNOTLBOWrapperPass.h"
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
static Opt<bool> Reorder
("ord", "Order the program input.", false, false);

// TODO: This should be change to a nicer interface.
static Opt<std::string> Allocator
("alloc", "Sets the allocator to be used. \
Default: dynprog. \
Options: dynprog; wpm; qubiter; wqubiter; random.", "dynprog", false);
static efd::Opt<std::string> Arch
("arch", "Name of the architechture, or a file \
with the connectivity graph.", "ibmqx2", false);

static void DumpToOutFile(QModule* qmod) {
    std::ofstream O(OutFilepath.getVal());
    qmod->print(O, Pretty.getVal());
    O.close();
}

int main(int argc, char** argv) {
    InitializeAllQbitAllocators();
    InitializeAllArchitectures();
    ParseArguments(argc, argv);

    std::string path = "./";
    std::string filename = InFilepath.getVal();

    auto lastslash = filename.find_last_of('/');

    if (lastslash != std::string::npos) {
        path = filename.substr(0, lastslash + 1);
        filename = filename.substr(lastslash + 1, std::string::npos);
    }

    QModule::sRef qmod = toShared(QModule::Parse(filename, path));

    if (qmod.get() != nullptr) {
        // Creating default passes.
        auto flattenPass = FlattenPass::Create();
        auto xbitUidPass = XbitToNumberWrapperPass::Create();

        flattenPass->run(qmod.get());

        if (Reorder.getVal()) {
            auto cnotlbo = CNOTLBOWrapperPass::Create();
            cnotlbo->run(qmod.get());
        }

        xbitUidPass->run(qmod.get());

        auto xbitToNumber = xbitUidPass->getData(); 

        // Architecture-dependent fragment.
        std::shared_ptr<efd::ArchGraph> graph;
        if (HasArchitecture(Arch.getVal())) {
            graph = CreateArchitecture(Arch.getVal());
        } else {
            graph = efd::ArchGraph::Read(Arch.getVal());
        }

        assert(xbitToNumber.getQSize() <= graph->size() &&
                "Using more qbits than the maximum.");

        auto allocator = efd::CreateQbitAllocator(Allocator.getVal(), graph);
        allocator->setInlineAll({ "cx", "u1", "u2", "u3", "intrinsic_rev_cx__",
                "intrinsic_swap__", "intrinsic_lcx__" });
        allocator->run(qmod.get());

        // Reversing the edges.
        auto revPass = ReverseEdgesPass::Create(graph);
        revPass->run(qmod.get());

        DumpToOutFile(qmod.get());
    }

    if (ShowStats.getVal())
        efd::PrintStats();

    return 0;
}
