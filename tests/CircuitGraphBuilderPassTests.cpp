#include "gtest/gtest.h"
#include "enfield/Transform/CircuitGraphBuilderPass.h"
#include "enfield/Support/uRefCast.h"
#include <string>
#include <unordered_map>

using namespace efd;

struct Checker {
    std::vector<bool> gused;
    std::vector<std::set<uint32_t>> lqused;
    std::vector<std::set<uint32_t>> lcused;
};

static void CheckCircuitGraph(std::string prog, Checker checker) {
    auto qmod = toShared(QModule::ParseString(prog));

    auto cgbp = CircuitGraphBuilderPass::Create();
    cgbp->run(qmod.get());

    auto cgraph = cgbp->getData();
    auto xbits = cgraph.size();

    for (uint32_t i = 0; i < xbits; ++i)
        if (checker.gused[i]) ASSERT_FALSE(cgraph[i] == nullptr);
        else ASSERT_TRUE(cgraph[i] == nullptr);

    bool stop;
    auto marked = std::vector<bool>(xbits, false);
    auto reached = std::unordered_map<Node::Ref, uint32_t>();

    do {
        stop = true;
        std::set<CircuitNode*> completed;

        for (uint32_t i = 0; i < xbits; ++i) {
            if (cgraph[i] && !marked[i]) {
                marked[i] = true;
                if (reached.find(cgraph[i]->node) == reached.end())
                    reached[cgraph[i]->node] = cgraph[i]->qargsid.size() +
                        cgraph[i]->cargsid.size();
                --reached[cgraph[i]->node];
            }
        }

        for (uint32_t i = 0; i < xbits; ++i) {
            if (cgraph[i] && !reached[cgraph[i]->node]) {
                completed.insert(cgraph[i]);
                marked[i] = false;
                cgraph[i] = cgraph[i]->child[i];
            }

            if (cgraph[i]) stop = false;
        }

        for (auto cnode : completed) {
            bool found = false;

            uint32_t i = 0;
            for (uint32_t e = checker.lqused.size(); i < e; ++i) {
                if (cnode->qargsid == checker.lqused[i] && cnode->cargsid == checker.lcused[i]) {
                    found = true;
                    checker.lqused.erase(checker.lqused.begin() + i);
                    checker.lcused.erase(checker.lcused.begin() + i);
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

        CheckCircuitGraph(program, Checker {{ 1, 1, 0, 0, 0 }, {{ 0, 1 }}, {{}}});
    }
    {
        const std::string program =
"\
OPENQASM 2.0;\
include \"qelib1.inc\";\
qreg q[5];\
h q[4];\
";

        CheckCircuitGraph(program, Checker {{ 0, 0, 0, 0, 1 }, {{ 4 }}, {{}}});
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

        CheckCircuitGraph(program, Checker {{ 0, 0, 1, 0, 0, 0, 0, 1, 0, 0 }, {{ 2 }}, {{ 7 }}});
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
                {{ 0, 1 }, { 3, 2 }, { 4 }}, {{}, {}, {}}});
        CheckCircuitGraph(program, Checker {{ 1, 1, 1, 1, 1 },
                {{ 0, 1 }, { 4 }, { 3, 2 }}, {{}, {}, {}}});
        CheckCircuitGraph(program, Checker {{ 1, 1, 1, 1, 1 },
                {{ 3, 2 }, { 0, 1 }, { 4 }}, {{}, {}, {}}});
        CheckCircuitGraph(program, Checker {{ 1, 1, 1, 1, 1 },
                {{ 3, 2 }, { 4 }, { 0, 1 }}, {{}, {}, {}}});
        CheckCircuitGraph(program, Checker {{ 1, 1, 1, 1, 1 },
                {{ 4 }, { 0, 1 }, { 3, 2 }}, {{}, {}, {}}});
        CheckCircuitGraph(program, Checker {{ 1, 1, 1, 1, 1 },
                {{ 4 }, { 3, 2 }, { 0, 1 }}, {{}, {}, {}}});
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
                {{ 4 }, { 2, 1 }, { 3 }, { 0 }}, {{}, {}, {}, {}}});
        CheckCircuitGraph(program, Checker {{ 1, 1, 1, 1, 1 },
                {{ 4 }, { 2, 1 }, { 0 }, { 3 }}, {{}, {}, {}, {}}});
        CheckCircuitGraph(program, Checker {{ 1, 1, 1, 1, 1 },
                {{ 4 }, { 3 }, { 2, 1 }, { 0 }}, {{}, {}, {}, {}}});
        CheckCircuitGraph(program, Checker {{ 1, 1, 1, 1, 1 },
                {{ 4 }, { 3 }, { 0 }, { 2, 1 }}, {{}, {}, {}, {}}});
        CheckCircuitGraph(program, Checker {{ 1, 1, 1, 1, 1 },
                {{ 4 }, { 0 }, { 3 }, { 2, 1 }}, {{}, {}, {}, {}}});
        CheckCircuitGraph(program, Checker {{ 1, 1, 1, 1, 1 },
                {{ 4 }, { 0 }, { 2, 1 }, { 3 }}, {{}, {}, {}, {}}});
        CheckCircuitGraph(program, Checker {{ 1, 1, 1, 1, 1 },
                {{ 3 }, { 2, 1 }, { 4 }, { 0 }}, {{}, {}, {}, {}}});
        CheckCircuitGraph(program, Checker {{ 1, 1, 1, 1, 1 },
                {{ 3 }, { 2, 1 }, { 0 }, { 4 }}, {{}, {}, {}, {}}});
        CheckCircuitGraph(program, Checker {{ 1, 1, 1, 1, 1 },
                {{ 3 }, { 4 }, { 2, 1 }, { 0 }}, {{}, {}, {}, {}}});
        CheckCircuitGraph(program, Checker {{ 1, 1, 1, 1, 1 },
                {{ 3 }, { 4 }, { 0 }, { 2, 1 }}, {{}, {}, {}, {}}});
        CheckCircuitGraph(program, Checker {{ 1, 1, 1, 1, 1 },
                {{ 3 }, { 0 }, { 4 }, { 2, 1 }}, {{}, {}, {}, {}}});
        CheckCircuitGraph(program, Checker {{ 1, 1, 1, 1, 1 },
                {{ 3 }, { 0 }, { 2, 1 }, { 4 }}, {{}, {}, {}, {}}});
        CheckCircuitGraph(program, Checker {{ 1, 1, 1, 1, 1 },
                {{ 0 }, { 2, 1 }, { 3 }, { 4 }}, {{}, {}, {}, {}}});
        CheckCircuitGraph(program, Checker {{ 1, 1, 1, 1, 1 },
                {{ 0 }, { 2, 1 }, { 4 }, { 3 }}, {{}, {}, {}, {}}});
        CheckCircuitGraph(program, Checker {{ 1, 1, 1, 1, 1 },
                {{ 0 }, { 3 }, { 2, 1 }, { 4 }}, {{}, {}, {}, {}}});
        CheckCircuitGraph(program, Checker {{ 1, 1, 1, 1, 1 },
                {{ 0 }, { 3 }, { 4 }, { 2, 1 }}, {{}, {}, {}, {}}});
        CheckCircuitGraph(program, Checker {{ 1, 1, 1, 1, 1 },
                {{ 0 }, { 4 }, { 3 }, { 2, 1 }}, {{}, {}, {}, {}}});
        CheckCircuitGraph(program, Checker {{ 1, 1, 1, 1, 1 },
                {{ 0 }, { 4 }, { 2, 1 }, { 3 }}, {{}, {}, {}, {}}});
        CheckCircuitGraph(program, Checker {{ 1, 1, 1, 1, 1 },
                {{ 2, 1 }, { 4 }, { 3 }, { 0 }}, {{}, {}, {}, {}}});
        CheckCircuitGraph(program, Checker {{ 1, 1, 1, 1, 1 },
                {{ 2, 1 }, { 4 }, { 0 }, { 3 }}, {{}, {}, {}, {}}});
        CheckCircuitGraph(program, Checker {{ 1, 1, 1, 1, 1 },
                {{ 2, 1 }, { 3 }, { 4 }, { 0 }}, {{}, {}, {}, {}}});
        CheckCircuitGraph(program, Checker {{ 1, 1, 1, 1, 1 },
                {{ 2, 1 }, { 3 }, { 0 }, { 4 }}, {{}, {}, {}, {}}});
        CheckCircuitGraph(program, Checker {{ 1, 1, 1, 1, 1 },
                {{ 2, 1 }, { 0 }, { 4 }, { 3 }}, {{}, {}, {}, {}}});
        CheckCircuitGraph(program, Checker {{ 1, 1, 1, 1, 1 },
                {{ 2, 1 }, { 0 }, { 3 }, { 4 }}, {{}, {}, {}, {}}});
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
                {{ 1 }, { 2 }, { 3 }, { 0 }}, {{ 6 }, { 7 }, { 8 }, { 5 }}});
        CheckCircuitGraph(program, Checker {{ 1, 1, 1, 1, 0, 1, 1, 1, 1, 0 },
                {{ 1 }, { 2 }, { 0 }, { 3 }}, {{ 6 }, { 7 }, { 5 }, { 8 }}});
        CheckCircuitGraph(program, Checker {{ 1, 1, 1, 1, 0, 1, 1, 1, 1, 0 },
                {{ 1 }, { 3 }, { 2 }, { 0 }}, {{ 6 }, { 8 }, { 7 }, { 5 }}});
        CheckCircuitGraph(program, Checker {{ 1, 1, 1, 1, 0, 1, 1, 1, 1, 0 },
                {{ 1 }, { 3 }, { 0 }, { 2 }}, {{ 6 }, { 8 }, { 5 }, { 7 }}});
        CheckCircuitGraph(program, Checker {{ 1, 1, 1, 1, 0, 1, 1, 1, 1, 0 },
                {{ 1 }, { 0 }, { 3 }, { 2 }}, {{ 6 }, { 5 }, { 8 }, { 7 }}});
        CheckCircuitGraph(program, Checker {{ 1, 1, 1, 1, 0, 1, 1, 1, 1, 0 },
                {{ 1 }, { 0 }, { 2 }, { 3 }}, {{ 6 }, { 5 }, { 7 }, { 8 }}});
        CheckCircuitGraph(program, Checker {{ 1, 1, 1, 1, 0, 1, 1, 1, 1, 0 },
                {{ 3 }, { 2 }, { 1 }, { 0 }}, {{ 8 }, { 7 }, { 6 }, { 5 }}});
        CheckCircuitGraph(program, Checker {{ 1, 1, 1, 1, 0, 1, 1, 1, 1, 0 },
                {{ 3 }, { 2 }, { 0 }, { 1 }}, {{ 8 }, { 7 }, { 5 }, { 6 }}});
        CheckCircuitGraph(program, Checker {{ 1, 1, 1, 1, 0, 1, 1, 1, 1, 0 },
                {{ 3 }, { 1 }, { 2 }, { 0 }}, {{ 8 }, { 6 }, { 7 }, { 5 }}});
        CheckCircuitGraph(program, Checker {{ 1, 1, 1, 1, 0, 1, 1, 1, 1, 0 },
                {{ 3 }, { 1 }, { 0 }, { 2 }}, {{ 8 }, { 6 }, { 5 }, { 7 }}});
        CheckCircuitGraph(program, Checker {{ 1, 1, 1, 1, 0, 1, 1, 1, 1, 0 },
                {{ 3 }, { 0 }, { 1 }, { 2 }}, {{ 8 }, { 5 }, { 6 }, { 7 }}});
        CheckCircuitGraph(program, Checker {{ 1, 1, 1, 1, 0, 1, 1, 1, 1, 0 },
                {{ 3 }, { 0 }, { 2 }, { 1 }}, {{ 8 }, { 5 }, { 7 }, { 6 }}});
        CheckCircuitGraph(program, Checker {{ 1, 1, 1, 1, 0, 1, 1, 1, 1, 0 },
                {{ 0 }, { 2 }, { 3 }, { 1 }}, {{ 5 }, { 7 }, { 8 }, { 6 }}});
        CheckCircuitGraph(program, Checker {{ 1, 1, 1, 1, 0, 1, 1, 1, 1, 0 },
                {{ 0 }, { 2 }, { 1 }, { 3 }}, {{ 5 }, { 7 }, { 6 }, { 8 }}});
        CheckCircuitGraph(program, Checker {{ 1, 1, 1, 1, 0, 1, 1, 1, 1, 0 },
                {{ 0 }, { 3 }, { 2 }, { 1 }}, {{ 5 }, { 8 }, { 7 }, { 6 }}});
        CheckCircuitGraph(program, Checker {{ 1, 1, 1, 1, 0, 1, 1, 1, 1, 0 },
                {{ 0 }, { 3 }, { 1 }, { 2 }}, {{ 5 }, { 8 }, { 6 }, { 7 }}});
        CheckCircuitGraph(program, Checker {{ 1, 1, 1, 1, 0, 1, 1, 1, 1, 0 },
                {{ 0 }, { 1 }, { 3 }, { 2 }}, {{ 5 }, { 6 }, { 8 }, { 7 }}});
        CheckCircuitGraph(program, Checker {{ 1, 1, 1, 1, 0, 1, 1, 1, 1, 0 },
                {{ 0 }, { 1 }, { 2 }, { 3 }}, {{ 5 }, { 6 }, { 7 }, { 8 }}});
        CheckCircuitGraph(program, Checker {{ 1, 1, 1, 1, 0, 1, 1, 1, 1, 0 },
                {{ 2 }, { 1 }, { 3 }, { 0 }}, {{ 7 }, { 6 }, { 8 }, { 5 }}});
        CheckCircuitGraph(program, Checker {{ 1, 1, 1, 1, 0, 1, 1, 1, 1, 0 },
                {{ 2 }, { 1 }, { 0 }, { 3 }}, {{ 7 }, { 6 }, { 5 }, { 8 }}});
        CheckCircuitGraph(program, Checker {{ 1, 1, 1, 1, 0, 1, 1, 1, 1, 0 },
                {{ 2 }, { 3 }, { 1 }, { 0 }}, {{ 7 }, { 8 }, { 6 }, { 5 }}});
        CheckCircuitGraph(program, Checker {{ 1, 1, 1, 1, 0, 1, 1, 1, 1, 0 },
                {{ 2 }, { 3 }, { 0 }, { 1 }}, {{ 7 }, { 8 }, { 5 }, { 6 }}});
        CheckCircuitGraph(program, Checker {{ 1, 1, 1, 1, 0, 1, 1, 1, 1, 0 },
                {{ 2 }, { 0 }, { 1 }, { 3 }}, {{ 7 }, { 5 }, { 6 }, { 8 }}});
        CheckCircuitGraph(program, Checker {{ 1, 1, 1, 1, 0, 1, 1, 1, 1, 0 },
                {{ 2 }, { 0 }, { 3 }, { 1 }}, {{ 7 }, { 5 }, { 8 }, { 6 }}});
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
                        { 0 },
                        { 1 },
                        { 1 },
                        { 1 },
                        { 2 },
                        { 2 },
                        { 2 },
                        { 2 },
                        { 2 },
                        { 3 },
                        { 3 },
                        { 3 },
                        { 3 },
                        { 3 },
                        { 3 },
                        { 3 },
                        { 3 },
                        { 3 }
                    },
                    {
                        {}, {}, {}, {},
                        {},
                        { 4 },
                        { 4, 5, 6, 7 },
                        {},
                        { 5 },
                        { 4, 5, 6, 7 },
                        { 4, 5, 6, 7 },
                        { 4, 5, 6, 7 },
                        {},
                        { 6 },
                        { 4, 5, 6, 7 },
                        { 4, 5, 6, 7 },
                        { 4, 5, 6, 7 },
                        { 4, 5, 6, 7 },
                        { 4, 5, 6, 7 },
                        { 4, 5, 6, 7 },
                        { 4, 5, 6, 7 },
                        {},
                        { 7 }
                    }
                });
    }
}
