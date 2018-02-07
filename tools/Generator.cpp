#include "enfield/Transform/QModule.h"
#include "enfield/Arch/ArchGraph.h"
#include "enfield/Arch/Architectures.h"
#include "enfield/Support/CommandLine.h"
#include "enfield/Support/uRefCast.h"

#include <fstream>
#include <random>

static efd::Opt<unsigned> Deps
("deps", "Number of dependencies.", 0, true);
static efd::Opt<unsigned> Vertices
("vert", "Number of vertices to be used.", 0, true);
static efd::Opt<std::string> Out
("o", "Name of the output file.", "/dev/stdout", false);

int main(int argc, char **argv) {
    efd::InitializeAllArchitectures();
    efd::ParseArguments(argc, argv);

    auto nqbts = Vertices.getVal();
    auto qmod = efd::QModule::Create();

    auto regId = efd::NDId::Create("q");

    qmod->insertReg(efd::NDRegDecl::CreateQ(
                efd::uniqueCastForward<efd::NDId>(regId->clone()),
                efd::NDInt::Create(std::to_string(nqbts))));

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

        auto lhs = efd::NDIdRef::Create(
                efd::uniqueCastForward<efd::NDId>(regId->clone()),
                efd::NDInt::Create(std::to_string(edge.first)));
        auto rhs = efd::NDIdRef::Create(
                efd::uniqueCastForward<efd::NDId>(regId->clone()),
                efd::NDInt::Create(std::to_string(edge.second)));

        auto qargs = efd::NDList::Create();
        qargs->addChild(lhs->clone());
        qargs->addChild(rhs->clone());

        auto cx = efd::NDQOpGen::Create(
                efd::NDId::Create("cx"),
                efd::NDList::Create(),
                std::move(qargs));

        qmod->insertStatementLast(std::move(cx));
    }

    qmod->print(o, true);
    o.close();
    return 0;
}
