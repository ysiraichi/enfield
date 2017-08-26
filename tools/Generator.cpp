#include "enfield/Transform/QModule.h"
#include "enfield/Arch/ArchGraph.h"
#include "enfield/Arch/Architectures.h"
#include "enfield/Support/CommandLine.h"

#include <fstream>
#include <random>

static efd::Opt<unsigned> Deps
("deps", "Number of dependencies.", 0, true);
static efd::Opt<std::string> Arch
("arch", "Name of the architechture, or a file \
with the connectivity graph.", "IBMQX2", false);
static efd::Opt<std::string> Out
("o", "Name of the output file.", "/dev/stdout", false);

int main(int argc, char **argv) {
    efd::ParseArguments(argc, argv);

    efd::ArchGraph* arch;
    if (Arch.getVal() == "IBMQX2") {
        arch = efd::ArchIBMQX2::Create().release();
    } else {
        arch = efd::ArchGraph::Read(Arch.getVal()).release();
    }

    auto qmod = efd::QModule::Create();
    std::ofstream o(Out.getVal());
    unsigned size = arch->size();

    std::random_device rd;
    std::mt19937 rng(rd());
    std::uniform_int_distribution<unsigned> runit(0, size-1);

    for (unsigned i = 0, e = Deps.getVal(); i < e; ++i) {
        unsigned u, v;

        do {
            u = runit(rng);
            v = runit(rng);
        } while (u == v);

        std::string lhs = arch->getSId(u);
        std::string rhs = arch->getSId(v);

        auto cx = efd::NDQOpCX::Create(efd::NDId::Create(lhs), efd::NDId::Create(rhs));
        qmod->insertStatementLast(std::move(cx));
    }

    qmod->print(o, true);
    o.close();
    return 0;
}
