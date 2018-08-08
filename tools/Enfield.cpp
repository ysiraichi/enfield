#include "enfield/Support/CommandLine.h"
#include "enfield/Transform/QModule.h"
#include "enfield/Transform/Driver.h"
#include "enfield/Transform/PassCache.h"
#include "enfield/Transform/IntrinsicGateCostPass.h"
#include "enfield/Transform/QModuleQualityEvalPass.h"
#include "enfield/Transform/InlineAllPass.h"
#include "enfield/Transform/ReverseEdgesPass.h"
#include "enfield/Transform/Allocators/Allocators.h"
#include "enfield/Arch/Architectures.h"
#include "enfield/Support/Stats.h"
#include "enfield/Support/Defs.h"

#include <fstream>
#include <cassert>
#include <cctype>
#include <sstream>
#include <map>

using namespace efd;

typedef std::map<std::string, uint32_t> MapGateUInt;

namespace efd {
    template <> std::string Opt<MapGateUInt>::getStringVal() {
        std::string str;
        for (auto& pair : mVal)
            str += pair.first + ":" + std::to_string(pair.second) + " ";
        return str;
    }

    template <> struct ParseOptTrait<MapGateUInt> {
        static void Run(Opt<MapGateUInt>* opt, std::vector<std::string> args) {
            std::string sVector = args[0];
            std::istringstream iVector(sVector);

            for (std::string str; iVector >> str;) {
                std::size_t i = str.find(':');
                int32_t l = i, r = i;

                if (i != std::string::npos) {
                    while (l >= 0 && !isspace(str[l])) --l;
                    while (r < (int32_t) str.length() && !isspace(str[r])) ++r;
                    ++l; --r;
                }

                std::string key = str.substr(l, i - l);
                std::string val = str.substr(i + 1, r - i);
                opt->mVal[key] = std::stoull(val);
            }
        }
    };
}

static efd::Opt<MapGateUInt> GateWeights
("-gate-w",
"Cost of using each basis gate. \
Should be specified as <gate>:<w> between quotes.",
{{"U", 1}, {"CX", 10}}, false);

static Opt<std::string> InFilepath
("i", "The input file.", "/dev/stdin", true);
static Opt<std::string> OutFilepath
("o", "The output file.", "/dev/stdout", false);
static Opt<std::string> ArchFilepath
("arch-file", "An input file for using a custom architecture.", "", false);

static Opt<bool> NoPretty
("-no-pretty", "Print in a pretty format (negation).", false, false);
static Opt<bool> ShowStats
("stats", "Print statistical data collected.", false, false);
static Opt<bool> Reorder
("ord", "Order the program input.", false, false);
static Opt<bool> NoVerify
("-no-verify", "Verify the compiled code (negation).", false, false);
static Opt<bool> Force
("f", "Compile and print the module, even if verification fails.", false, false);

static Opt<EnumAllocator> Alloc
("alloc", "Sets the allocator to be used.", Allocator::Q_dynprog, false);
static efd::Opt<EnumArchitecture> Arch
("arch", "Name of the architechture, or a file with the connectivity graph.",
Architecture::A_ibmqx2, false);

static Opt<std::string> PrintDepGraphFile
("-print-depgraph", "Choose a file to print the dependency graph.", "", false);
static Opt<std::string> PrintArchGraphFile
("-print-archgraph", "Choose a file to print the architecture graph.", "", false);

static efd::Stat<uint32_t> TotalCost
("TotalCost", "Total cost after allocating the qubits.");
static efd::Stat<uint32_t> Depth
("Depth", "Total depth after allocating the qubits.");
static efd::Stat<uint32_t> Gates
("Gates", "Total number of gates after allocating the qubits.");
static efd::Stat<uint32_t> WeightedCost
("WeightedCost", "Total weighted cost after allocating the qubits.");

static void DumpToOutFile(QModule::Ref qmod) {
    std::ofstream O(OutFilepath.getVal());
    PrintToStream(qmod, O, !NoPretty.getVal());
    O.close();
}

static void ComputeStats(QModule::Ref qmod, ArchGraph::sRef archGraph) {
    TotalCost = PassCache::Get<IntrinsicGateCostPass>(qmod)
                    ->getData();

    std::vector<std::string> basisGates;
    for (auto& pair : GateWeights.getVal()) {
        basisGates.push_back(pair.first);
    }

    auto inlinePass = InlineAllPass::Create(basisGates);
    auto reversePass = ReverseEdgesPass::Create(archGraph);
    auto qualityPass = QModuleQualityEvalPass::Create(GateWeights.getVal());
    PassCache::Run(qmod, inlinePass.get());
    PassCache::Run(qmod, reversePass.get());
    PassCache::Run(qmod, inlinePass.get());
    PassCache::Run(qmod, qualityPass.get());
    auto &quality = qualityPass->getData();

    Depth = quality.mDepth;
    Gates = quality.mGates;
    WeightedCost = quality.mWeightedCost;
}

int main(int argc, char** argv) {
    InitializeAllQbitAllocators();
    InitializeAllArchitectures();

    ParseArguments(argc, argv);
    QModule::uRef qmod = ParseFile(InFilepath.getVal());

    if (qmod.get() != nullptr) {
        ArchGraph::sRef archGraph;
        if (!ArchFilepath.isParsed() && HasArchitecture(Arch.getVal())) {
            archGraph = CreateArchitecture(Arch.getVal());
        } else if (ArchFilepath.isParsed()) {
            archGraph = ArchGraph::Read(ArchFilepath.getVal());
        } else {
            ERR << "Architecture: " << Arch.getVal().getStringValue()
                << " not found." << std::endl;
        }

        if (PrintArchGraphFile.isParsed()) {
            std::ofstream ofs(PrintArchGraphFile.getVal());
            ofs << archGraph->dotify() << std::endl;
            ofs.close();
        }

        if (PrintDepGraphFile.isParsed()) {
            std::ofstream ofs(PrintDepGraphFile.getVal());
            PrintDependencyGraph(qmod.get(), ofs);
            ofs.close();
        }

        CompilationSettings settings {
            archGraph,
            Alloc.getVal(),
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
            !NoVerify.getVal(),
            Force.getVal()
        };

        qmod.reset(Compile(std::move(qmod), settings).release());

        if (qmod.get() != nullptr)
            DumpToOutFile(qmod.get());

        ComputeStats(qmod.get(), archGraph);
    }

    if (ShowStats.getVal())
        efd::PrintStats();

    return 0;
}
