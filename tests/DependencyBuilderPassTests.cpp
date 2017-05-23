#include "gtest/gtest.h"

#include "enfield/Analysis/QModule.h"
#include "enfield/Transform/DependencyBuilderPass.h"

#include <string>
#include <unordered_map>

using namespace efd;

TEST(QbitToNumberPassTests, WholeProgramTest) {
    {
        const std::string program = \
"\
qreg q[5];\
";

        std::unique_ptr<QModule> qmod = QModule::ParseString(program);
        QbitToNumberPass* pass = QbitToNumberPass::Create();
        qmod->runPass(pass);

        ASSERT_DEATH({ pass->getUId("q"); }, "Id not found");
        ASSERT_TRUE(pass->getUId("q[0]") == 0);
        ASSERT_TRUE(pass->getUId("q[1]") == 1);
        ASSERT_TRUE(pass->getUId("q[2]") == 2);
        ASSERT_TRUE(pass->getUId("q[3]") == 3);
        ASSERT_TRUE(pass->getUId("q[4]") == 4);
    }

    {
        const std::string program = \
"\
gate mygate(a, b, c) x, y, z {\
    cx x, y; cx y, z;\
}\
";

        std::unique_ptr<QModule> qmod = QModule::ParseString(program);
        QbitToNumberPass* pass = QbitToNumberPass::Create();
        qmod->runPass(pass);

        NDGateDecl* gate = qmod->getQGate("mygate");
        ASSERT_FALSE(gate == nullptr);

        ASSERT_DEATH({ pass->getUId("mygate"); }, "Id not found");
        ASSERT_DEATH({ pass->getUId("x"); }, "Id not found");
        ASSERT_DEATH({ pass->getUId("y"); }, "Id not found");
        ASSERT_DEATH({ pass->getUId("z"); }, "Id not found");

        ASSERT_TRUE(pass->getUId("x", gate) == 0);
        ASSERT_TRUE(pass->getUId("y", gate) == 1);
        ASSERT_TRUE(pass->getUId("z", gate) == 2);
    }

    {
        const std::string program = \
"\
gate id a {\
}\
gate cnot a, b {\
    cx a, b;\
}\
qreg q[5];\
creg c[5];\
id q[0];\
id q[1];\
cnot q[0], q;\
measure q[0] -> c[0];\
measure q[1] -> c[1];\
measure q[2] -> c[2];\
measure q[3] -> c[3];\
measure q[4] -> c[4];\
";

        std::unique_ptr<QModule> qmod = QModule::ParseString(program);
        QbitToNumberPass* pass = QbitToNumberPass::Create();
        qmod->runPass(pass);

        NDGateDecl* idGate = qmod->getQGate("id");
        ASSERT_FALSE(idGate == nullptr);
        NDGateDecl* cnotGate = qmod->getQGate("cnot");
        ASSERT_FALSE(cnotGate == nullptr);

        ASSERT_DEATH({ pass->getUId("id"); }, "Id not found");
        ASSERT_DEATH({ pass->getUId("cnot"); }, "Id not found");
        ASSERT_DEATH({ pass->getUId("a"); }, "Id not found");
        ASSERT_DEATH({ pass->getUId("b"); }, "Id not found");
        ASSERT_DEATH({ pass->getUId("q"); }, "Id not found");
        ASSERT_DEATH({ pass->getUId("c"); }, "Id not found");
        ASSERT_DEATH({ pass->getUId("c[0]"); }, "Id not found");
        ASSERT_DEATH({ pass->getUId("c[1]"); }, "Id not found");
        ASSERT_DEATH({ pass->getUId("c[2]"); }, "Id not found");
        ASSERT_DEATH({ pass->getUId("c[3]"); }, "Id not found");
        ASSERT_DEATH({ pass->getUId("c[4]"); }, "Id not found");

        ASSERT_TRUE(pass->getUId("a", idGate) == 0);
        ASSERT_TRUE(pass->getUId("a", cnotGate) == 0);
        ASSERT_TRUE(pass->getUId("b", cnotGate) == 1);
        ASSERT_TRUE(pass->getUId("q[0]") == 0);
        ASSERT_TRUE(pass->getUId("q[1]") == 1);
        ASSERT_TRUE(pass->getUId("q[2]") == 2);
        ASSERT_TRUE(pass->getUId("q[3]") == 3);
        ASSERT_TRUE(pass->getUId("q[4]") == 4);
    }
}

TEST(DependencyBuilderPassTest, GateDependenciesTest) {
    {
        const std::string program = \
"\
gate cnot x, y {\
    CX x, y;\
}\
";

        std::unique_ptr<QModule> qmod = QModule::ParseString(program);
        DependencyBuilderPass* pass = DependencyBuilderPass::Create(qmod.get());
        qmod->runPass(pass);

        unsigned x = 0;
        unsigned y = 1;

        NDGateDecl* gate = qmod->getQGate("cnot");
        ASSERT_FALSE(gate == nullptr);

        DependencyBuilderPass::DepsSet deps = pass->getDependencies(gate);
        // Has only one parallel dependency.
        ASSERT_EQ(deps.size(), 1);
        // The only paralel dependency has only one parallel dependency.
        ASSERT_EQ(deps[0].getSize(), 1);
        ASSERT_EQ(deps[0][0].mFrom, x);
        ASSERT_EQ(deps[0][0].mTo, y);

        ASSERT_TRUE(deps[0].mCallPoint == nullptr);
    }

    {
        const std::string program = \
"\
gate cx x, y {\
    CX x, y;\
}\
gate cnot x, y {\
    cx x, y;\
}\
";

        std::unique_ptr<QModule> qmod = QModule::ParseString(program);
        DependencyBuilderPass* pass = DependencyBuilderPass::Create(qmod.get());
        qmod->runPass(pass);

        unsigned x = 0;
        unsigned y = 1;

        NDGateDecl* cxGate = qmod->getQGate("cx");
        ASSERT_FALSE(cxGate == nullptr);
        NDGateDecl* cnotGate = qmod->getQGate("cnot");
        ASSERT_FALSE(cnotGate == nullptr);

        DependencyBuilderPass::DepsSet cxDeps = pass->getDependencies(cxGate);
        DependencyBuilderPass::DepsSet cnotDeps = pass->getDependencies(cnotGate);

        // Has only one parallel dependency.
        ASSERT_EQ(cxDeps.size(), 1);
        // The only parallel dependency has only one parallel dependency.
        ASSERT_EQ(cxDeps[0].getSize(), 1);
        ASSERT_EQ(cxDeps[0][0].mFrom, x);
        ASSERT_EQ(cxDeps[0][0].mTo, y);

        ASSERT_TRUE(cxDeps[0].mCallPoint == nullptr);

        // Has only one parallel dependency.
        ASSERT_EQ(cnotDeps.size(), 1);
        // The only parallel dependency has only one parallel dependency.
        ASSERT_EQ(cnotDeps[0].getSize(), 1);
        ASSERT_EQ(cnotDeps[0][0].mFrom, x);
        ASSERT_EQ(cnotDeps[0][0].mTo, y);

        ASSERT_FALSE(cnotDeps[0].mCallPoint == nullptr);
    }
}

TEST(DependencyBuilderPassTest, ProgramDependenciesTest) {
    {
        const std::string program = \
"\
qreg q[2];\
CX q[0], q[1];\
";

        std::unique_ptr<QModule> qmod = QModule::ParseString(program);
        DependencyBuilderPass* pass = DependencyBuilderPass::Create(qmod.get());
        qmod->runPass(pass);

        unsigned q0 = 0;
        unsigned q1 = 1;

        DependencyBuilderPass::DepsSet deps = pass->getDependencies();

        // Has only one parallel dependency.
        ASSERT_EQ(deps.size(), 1);
        // The only parallel dependency has only one parallel dependency.
        ASSERT_EQ(deps[0].getSize(), 1);
        ASSERT_EQ(deps[0][0].mFrom, q0);
        ASSERT_EQ(deps[0][0].mTo, q1);

        ASSERT_TRUE(deps[0].mCallPoint == nullptr);
    }

    {
        const std::string program = \
"\
include \"files/qelib1.inc\";\
gate majority a, b, c {\
cx c, b;\
cx c, a;\
ccx a, b, c;\
}\
gate unmaj a, b, c {\
ccx a, b, c;\
cx c, a;\
cx a, b;\
}\
gate add4 a0, a1, a2, a3, b0, b1, b2, b3, cin, cout {\
majority cin, b0, a0;\
majority a0, b1, a1;\
majority a1, b2, a2;\
majority a2, b3, a3;\
cx a3, cout;\
unmaj a2, b3, a3;\
unmaj a1, b2, a2;\
unmaj a0, b1, a1;\
unmaj cin, b0, a0;\
}\
qreg carry[2];\
qreg a[8];\
qreg b[8];\
creg ans[8];\
creg carryout[1];\
x a[0];\
x b[0];\
x b[1];\
x b[2];\
x b[3];\
x b[4];\
x b[5];\
x b[6];\
x b[7];\
add4 a[0], a[1], a[2], a[3], b[0], b[1], b[2], b[3], carry[0], carry[1];\
add4 a[4], a[5], a[6], a[7], b[4], b[5], b[6], b[7], carry[1], carry[0];\
measure b[0] -> ans[0];\
measure b[1] -> ans[1];\
measure b[2] -> ans[2];\
measure b[3] -> ans[3];\
measure b[4] -> ans[4];\
measure b[5] -> ans[5];\
measure b[6] -> ans[6];\
measure b[7] -> ans[7];\
measure carry[0] -> carryout[0];\
";

        std::unique_ptr<QModule> qmod = QModule::ParseString(program);
        DependencyBuilderPass* pass = DependencyBuilderPass::Create(qmod.get());
        qmod->runPass(pass);


        std::unordered_map<std::string, std::pair<unsigned, unsigned>> gatesInfo = {
            { "ccx", { 6, 6 } }, { "cx", { 1, 1 } }, { "x", { 0, 0 } }, { "majority", { 3, 8 } }, { "add4", { 9, 65 } }, { "unmaj", { 3, 8 } }
        };

        DependencyBuilderPass::DepsSet deps;
        for (auto pair : gatesInfo) {
            NDGateDecl* gate = qmod->getQGate(pair.first);
            ASSERT_FALSE(gate == nullptr);

            deps = pass->getDependencies(gate);
            ASSERT_EQ(deps.size(), pair.second.first);

            unsigned sum = 0;
            for (auto v : deps) sum += v.getSize();
            ASSERT_EQ(sum, pair.second.second);
        }
    }

}
