#include "gtest/gtest.h"
#include "enfield/Transform/CircuitGraphBuilderPass.h"
#include "enfield/Support/uRefCast.h"
#include <string>
#include <unordered_map>

using namespace efd;

struct Checker {
    std::vector<bool> gused;
    std::vector<std::set<uint32_t>> used;
};

static void CheckCircuitGraph(std::string prog, Checker checker) {
    auto qmod = toShared(QModule::ParseString(prog));

    auto cgbp = CircuitGraphBuilderPass::Create();
    cgbp->run(qmod.get());

    auto cgraph = cgbp->getData();
    auto it = cgraph.build_iterator();
    auto xbits = cgraph.size();

    for (uint32_t i = 0; i < xbits; ++i) {
        it.next(i);
        if (checker.gused[i]) ASSERT_TRUE(it[i]->isGateNode());
        else ASSERT_FALSE(it[i]->isGateNode());
    }

    bool stop;
    auto marked = std::vector<bool>(xbits, false);
    auto reached = std::unordered_map<Node::Ref, uint32_t>();

    do {
        stop = true;
        std::set<CircuitGraph::CircuitNode::sRef> completed;

        for (uint32_t i = 0; i < xbits; ++i) {
            if (it[i]->isGateNode() && !marked[i]) {
                auto node = it[i]->node();
                marked[i] = true;

                if (reached.find(node) == reached.end())
                    reached[node] = it[i]->numberOfXbits();
                --reached[node];
            }
        }

        for (uint32_t i = 0; i < xbits; ++i) {
            auto node = it[i]->node();

            if (it[i]->isGateNode() && !reached[node]) {
                completed.insert(it[i]);
                marked[i] = false;
                it.next(i);
            }

            if (!it[i]->isOutputNode()) stop = false;
        }

        for (auto cnode : completed) {
            bool found = false;

            uint32_t i = 0;
            for (uint32_t e = checker.used.size(); i < e; ++i) {
                std::set<uint32_t> idSet;

                for (uint32_t id: cnode->getXbitsId()) {
                    idSet.insert(id);
                }

                if (idSet == checker.used[i]) {
                    found = true;
                    checker.used.erase(checker.used.begin() + i);
                    break;
                }
            }

            ASSERT_TRUE(found);
        }

    } while (!stop);
}

TEST(CircuitGraphBuilderPassTests, OneOperationProgram) {
    {
        const std::string program =
"\
OPENQASM 2.0;\
include \"qelib1.inc\";\
qreg q[5];\
cx q[0], q[1];\
";

        CheckCircuitGraph(program, Checker {{ 1, 1, 0, 0, 0 }, {{ 0, 1 }}});
    }
    {
        const std::string program =
"\
OPENQASM 2.0;\
include \"qelib1.inc\";\
qreg q[5];\
h q[4];\
";

        CheckCircuitGraph(program, Checker {{ 0, 0, 0, 0, 1 }, {{ 4 }}});
    }
    {
        const std::string program =
"\
OPENQASM 2.0;\
include \"qelib1.inc\";\
qreg q[5];\
creg c[5];\
measure q[2] -> c[2];\
";

        CheckCircuitGraph(program, Checker {{ 0, 0, 1, 0, 0, 0, 0, 1, 0, 0 }, {{ 2, 7}}});
    }
}

TEST(CircuitGraphBuilderPassTests, IndependentOperationsProgram) {
    {
        const std::string program =
"\
OPENQASM 2.0;\
include \"qelib1.inc\";\
qreg q[5];\
cx q[0], q[1];\
cx q[3], q[2];\
h q[4];\
";

        CheckCircuitGraph(program, Checker {{ 1, 1, 1, 1, 1 },
                {{ 0, 1 }, { 3, 2 }, { 4 }}});
        CheckCircuitGraph(program, Checker {{ 1, 1, 1, 1, 1 },
                {{ 0, 1 }, { 4 }, { 3, 2 }}});
        CheckCircuitGraph(program, Checker {{ 1, 1, 1, 1, 1 },
                {{ 3, 2 }, { 0, 1 }, { 4 }}});
        CheckCircuitGraph(program, Checker {{ 1, 1, 1, 1, 1 },
                {{ 3, 2 }, { 4 }, { 0, 1 }}});
        CheckCircuitGraph(program, Checker {{ 1, 1, 1, 1, 1 },
                {{ 4 }, { 0, 1 }, { 3, 2 }}});
        CheckCircuitGraph(program, Checker {{ 1, 1, 1, 1, 1 },
                {{ 4 }, { 3, 2 }, { 0, 1 }}});
    }
    {
        const std::string program =
"\
OPENQASM 2.0;\
include \"qelib1.inc\";\
qreg q[5];\
h q[4];\
cx q[2], q[1];\
h q[3];\
h q[0];\
";

        CheckCircuitGraph(program, Checker {{ 1, 1, 1, 1, 1 },
                {{ 4 }, { 2, 1 }, { 3 }, { 0 }}});
        CheckCircuitGraph(program, Checker {{ 1, 1, 1, 1, 1 },
                {{ 4 }, { 2, 1 }, { 0 }, { 3 }}});
        CheckCircuitGraph(program, Checker {{ 1, 1, 1, 1, 1 },
                {{ 4 }, { 3 }, { 2, 1 }, { 0 }}});
        CheckCircuitGraph(program, Checker {{ 1, 1, 1, 1, 1 },
                {{ 4 }, { 3 }, { 0 }, { 2, 1 }}});
        CheckCircuitGraph(program, Checker {{ 1, 1, 1, 1, 1 },
                {{ 4 }, { 0 }, { 3 }, { 2, 1 }}});
        CheckCircuitGraph(program, Checker {{ 1, 1, 1, 1, 1 },
                {{ 4 }, { 0 }, { 2, 1 }, { 3 }}});
        CheckCircuitGraph(program, Checker {{ 1, 1, 1, 1, 1 },
                {{ 3 }, { 2, 1 }, { 4 }, { 0 }}});
        CheckCircuitGraph(program, Checker {{ 1, 1, 1, 1, 1 },
                {{ 3 }, { 2, 1 }, { 0 }, { 4 }}});
        CheckCircuitGraph(program, Checker {{ 1, 1, 1, 1, 1 },
                {{ 3 }, { 4 }, { 2, 1 }, { 0 }}});
        CheckCircuitGraph(program, Checker {{ 1, 1, 1, 1, 1 },
                {{ 3 }, { 4 }, { 0 }, { 2, 1 }}});
        CheckCircuitGraph(program, Checker {{ 1, 1, 1, 1, 1 },
                {{ 3 }, { 0 }, { 4 }, { 2, 1 }}});
        CheckCircuitGraph(program, Checker {{ 1, 1, 1, 1, 1 },
                {{ 3 }, { 0 }, { 2, 1 }, { 4 }}});
        CheckCircuitGraph(program, Checker {{ 1, 1, 1, 1, 1 },
                {{ 0 }, { 2, 1 }, { 3 }, { 4 }}});
        CheckCircuitGraph(program, Checker {{ 1, 1, 1, 1, 1 },
                {{ 0 }, { 2, 1 }, { 4 }, { 3 }}});
        CheckCircuitGraph(program, Checker {{ 1, 1, 1, 1, 1 },
                {{ 0 }, { 3 }, { 2, 1 }, { 4 }}});
        CheckCircuitGraph(program, Checker {{ 1, 1, 1, 1, 1 },
                {{ 0 }, { 3 }, { 4 }, { 2, 1 }}});
        CheckCircuitGraph(program, Checker {{ 1, 1, 1, 1, 1 },
                {{ 0 }, { 4 }, { 3 }, { 2, 1 }}});
        CheckCircuitGraph(program, Checker {{ 1, 1, 1, 1, 1 },
                {{ 0 }, { 4 }, { 2, 1 }, { 3 }}});
        CheckCircuitGraph(program, Checker {{ 1, 1, 1, 1, 1 },
                {{ 2, 1 }, { 4 }, { 3 }, { 0 }}});
        CheckCircuitGraph(program, Checker {{ 1, 1, 1, 1, 1 },
                {{ 2, 1 }, { 4 }, { 0 }, { 3 }}});
        CheckCircuitGraph(program, Checker {{ 1, 1, 1, 1, 1 },
                {{ 2, 1 }, { 3 }, { 4 }, { 0 }}});
        CheckCircuitGraph(program, Checker {{ 1, 1, 1, 1, 1 },
                {{ 2, 1 }, { 3 }, { 0 }, { 4 }}});
        CheckCircuitGraph(program, Checker {{ 1, 1, 1, 1, 1 },
                {{ 2, 1 }, { 0 }, { 4 }, { 3 }}});
        CheckCircuitGraph(program, Checker {{ 1, 1, 1, 1, 1 },
                {{ 2, 1 }, { 0 }, { 3 }, { 4 }}});
    }
    {
        const std::string program =
"\
OPENQASM 2.0;\
include \"qelib1.inc\";\
qreg q[5];\
creg c[5];\
measure q[0] -> c[0];\
measure q[1] -> c[1];\
measure q[2] -> c[2];\
measure q[3] -> c[3];\
";


        CheckCircuitGraph(program, Checker {{ 1, 1, 1, 1, 0, 1, 1, 1, 1, 0 },
                {{ 1, 6 }, { 2, 7 }, { 3, 8 }, { 0, 5 }}});
        CheckCircuitGraph(program, Checker {{ 1, 1, 1, 1, 0, 1, 1, 1, 1, 0 },
                {{ 1, 6 }, { 2, 7 }, { 0, 5 }, { 3, 8 }}});
        CheckCircuitGraph(program, Checker {{ 1, 1, 1, 1, 0, 1, 1, 1, 1, 0 },
                {{ 1, 6 }, { 3, 8 }, { 2, 7 }, { 0, 5 }}});
        CheckCircuitGraph(program, Checker {{ 1, 1, 1, 1, 0, 1, 1, 1, 1, 0 },
                {{ 1, 6 }, { 3, 8 }, { 0, 5 }, { 2, 7 }}});
        CheckCircuitGraph(program, Checker {{ 1, 1, 1, 1, 0, 1, 1, 1, 1, 0 },
                {{ 1, 6 }, { 0, 5 }, { 3, 8 }, { 2, 7 }}});
        CheckCircuitGraph(program, Checker {{ 1, 1, 1, 1, 0, 1, 1, 1, 1, 0 },
                {{ 1, 6 }, { 0, 5 }, { 2, 7 }, { 3, 8 }}});
        CheckCircuitGraph(program, Checker {{ 1, 1, 1, 1, 0, 1, 1, 1, 1, 0 },
                {{ 3, 8 }, { 2, 7 }, { 1, 6 }, { 0, 5 }}});
        CheckCircuitGraph(program, Checker {{ 1, 1, 1, 1, 0, 1, 1, 1, 1, 0 },
                {{ 3, 8 }, { 2, 7 }, { 0, 5 }, { 1, 6 }}});
        CheckCircuitGraph(program, Checker {{ 1, 1, 1, 1, 0, 1, 1, 1, 1, 0 },
                {{ 3, 8 }, { 1, 6 }, { 2, 7 }, { 0, 5 }}});
        CheckCircuitGraph(program, Checker {{ 1, 1, 1, 1, 0, 1, 1, 1, 1, 0 },
                {{ 3, 8 }, { 1, 6 }, { 0, 5 }, { 2, 7 }}});
        CheckCircuitGraph(program, Checker {{ 1, 1, 1, 1, 0, 1, 1, 1, 1, 0 },
                {{ 3, 8 }, { 0, 5 }, { 1, 6 }, { 2, 7 }}});
        CheckCircuitGraph(program, Checker {{ 1, 1, 1, 1, 0, 1, 1, 1, 1, 0 },
                {{ 3, 8 }, { 0, 5 }, { 2, 7 }, { 1, 6 }}});
        CheckCircuitGraph(program, Checker {{ 1, 1, 1, 1, 0, 1, 1, 1, 1, 0 },
                {{ 0, 5 }, { 2, 7 }, { 3, 8 }, { 1, 6 }}});
        CheckCircuitGraph(program, Checker {{ 1, 1, 1, 1, 0, 1, 1, 1, 1, 0 },
                {{ 0, 5 }, { 2, 7 }, { 1, 6 }, { 3, 8 }}});
        CheckCircuitGraph(program, Checker {{ 1, 1, 1, 1, 0, 1, 1, 1, 1, 0 },
                {{ 0, 5 }, { 3, 8 }, { 2, 7 }, { 1, 6 }}});
        CheckCircuitGraph(program, Checker {{ 1, 1, 1, 1, 0, 1, 1, 1, 1, 0 },
                {{ 0, 5 }, { 3, 8 }, { 1, 6 }, { 2, 7 }}});
        CheckCircuitGraph(program, Checker {{ 1, 1, 1, 1, 0, 1, 1, 1, 1, 0 },
                {{ 0, 5 }, { 1, 6 }, { 3, 8 }, { 2, 7 }}});
        CheckCircuitGraph(program, Checker {{ 1, 1, 1, 1, 0, 1, 1, 1, 1, 0 },
                {{ 0, 5 }, { 1, 6 }, { 2, 7 }, { 3, 8 }}});
        CheckCircuitGraph(program, Checker {{ 1, 1, 1, 1, 0, 1, 1, 1, 1, 0 },
                {{ 2, 7 }, { 1, 6 }, { 3, 8 }, { 0, 5 }}});
        CheckCircuitGraph(program, Checker {{ 1, 1, 1, 1, 0, 1, 1, 1, 1, 0 },
                {{ 2, 7 }, { 1, 6 }, { 0, 5 }, { 3, 8 }}});
        CheckCircuitGraph(program, Checker {{ 1, 1, 1, 1, 0, 1, 1, 1, 1, 0 },
                {{ 2, 7 }, { 3, 8 }, { 1, 6 }, { 0, 5 }}});
        CheckCircuitGraph(program, Checker {{ 1, 1, 1, 1, 0, 1, 1, 1, 1, 0 },
                {{ 2, 7 }, { 3, 8 }, { 0, 5 }, { 1, 6 }}});
        CheckCircuitGraph(program, Checker {{ 1, 1, 1, 1, 0, 1, 1, 1, 1, 0 },
                {{ 2, 7 }, { 0, 5 }, { 1, 6 }, { 3, 8 }}});
        CheckCircuitGraph(program, Checker {{ 1, 1, 1, 1, 0, 1, 1, 1, 1, 0 },
                {{ 2, 7 }, { 0, 5 }, { 3, 8 }, { 1, 6 }}});
    }
}

TEST(CircuitGraphBuilderPassTests, GenericProgram) {
    {
        const std::string program =
"\
OPENQASM 2.0;\
include \"qelib1.inc\";\
qreg q[4];\
creg c[4];\
h q[0];\
h q[1];\
h q[2];\
h q[3];\
h q[0];\
measure q[0] -> c[0];\
if(c==1) u1(pi/2) q[1];\
h q[1];\
measure q[1] -> c[1];\
if(c==1) u1(pi/4) q[2];\
if(c==2) u1(pi/2) q[2];\
if(c==3) u1(pi/2+pi/4) q[2];\
h q[2];\
measure q[2] -> c[2];\
if(c==1) u1(pi/8) q[3];\
if(c==2) u1(pi/4) q[3];\
if(c==3) u1(pi/4+pi/8) q[3];\
if(c==4) u1(pi/2) q[3];\
if(c==5) u1(pi/2+pi/8) q[3];\
if(c==6) u1(pi/2+pi/4) q[3];\
if(c==7) u1(pi/2+pi/4+pi/8) q[3];\
h q[3];\
measure q[3] -> c[3];\
";

        CheckCircuitGraph(program, Checker {
                    { 1, 1, 1, 1, 1, 1, 1, 1 },
                    {
                        { 0 }, { 1 }, { 2 }, { 3 },
                        { 0 },
                        { 0, 4 },
                        { 1, 4, 5, 6, 7 },
                        { 1 },
                        { 1, 5 },
                        { 2, 4, 5, 6, 7 },
                        { 2, 4, 5, 6, 7  },
                        { 2, 4, 5, 6, 7  },
                        { 2 },
                        { 2, 6 },
                        { 3, 4, 5, 6, 7  },
                        { 3, 4, 5, 6, 7  },
                        { 3, 4, 5, 6, 7  },
                        { 3, 4, 5, 6, 7  },
                        { 3, 4, 5, 6, 7  },
                        { 3, 4, 5, 6, 7  },
                        { 3, 4, 5, 6, 7  },
                        { 3 },
                        { 3, 7 }
                    }
                });
    }
}
