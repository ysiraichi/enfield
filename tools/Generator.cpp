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

    auto nqbts = arch->size();
    auto qmod = efd::QModule::Create();

    for (auto it = arch->reg_begin(), end = arch->reg_end(); it != end; ++it) {
        qmod->insertReg(efd::NDRegDecl::CreateQ(
                    efd::NDId::Create(it->first),
                    efd::NDInt::Create(std::to_string(it->second))));
    }

    std::ofstream o(Out.getVal());
    std::vector<std::pair<unsigned, unsigned>> edges;

    for (unsigned i = 0; i < nqbts; ++i) {
        for (unsigned j = 0; j < nqbts; ++j) {
            if (i != j) edges.push_back(std::make_pair(i, j));
        }
    }

    auto nedg = edges.size();

    std::random_device rd;
    std::mt19937 rng(rd());
    std::uniform_int_distribution<unsigned> runit(0, nedg - 1);

    for (unsigned i = 0, e = Deps.getVal(); i < e; ++i) {
        auto x = runit(rng);
        auto edge = edges[x];

        auto lhs = arch->getNode(edge.first);
        auto rhs = arch->getNode(edge.second);

        auto qargs = efd::NDList::Create();
        qargs->addChild(lhs->clone());
        qargs->addChild(rhs->clone());

        auto cx = efd::NDQOp::Create(
                efd::NDId::Create("cx"),
                efd::NDList::Create(),
                std::move(qargs));

        qmod->insertStatementLast(std::move(cx));
    }

    qmod->print(o, true);
    o.close();
    return 0;
}
