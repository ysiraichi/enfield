#include "enfield/Support/CommandLine.h"
#include "enfield/Transform/QModule.h"
#include "enfield/Transform/Driver.h"
#include "enfield/Transform/Allocators/Allocators.h"
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
static Opt<bool> Reorder
("ord", "Order the program input.", false, false);
static Opt<bool> Verify
("verify", "Verify the compiled code.", true, false);

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
    PrintToStream(qmod, O);
    O.close();
}

int main(int argc, char** argv) {
    InitializeAllQbitAllocators();
    InitializeAllArchitectures();

    ParseArguments(argc, argv);
    QModule::uRef qmod = ParseFile(InFilepath.getVal());

    if (qmod.get() != nullptr) {
        CompilationSettings settings {
            Arch.getVal(),
            Allocator.getVal(),
            {
                "intrinsic_swap__",
                "intrinsic_rev_cx__",
                "intrinsic_lcx__",
                "cx",
                "u1",
                "u2",
                "u3",
                "h"
            },
            Reorder.getVal(),
            Verify.getVal()
        };

        qmod.reset(Compile(std::move(qmod), settings).release());
        DumpToOutFile(qmod.get());
    }

    if (ShowStats.getVal())
        efd::PrintStats();

    return 0;
}
