
#include "gtest/gtest.h"

#include "enfield/Transform/QubitRemapPass.h"
#include "enfield/Transform/QModule.h"
#include "enfield/Support/RTTI.h"

#include <string>
#include <numeric>

using namespace efd;

TEST(QubitRemapPassTests, NoRemapTest) {
    {
        const std::string program =
"\
include \"qelib1.inc\";\
qreg q[5];\
CX q[0], q[1];\
";
        std::unique_ptr<QModule> qmod = QModule::ParseString(program);
        
        Mapping m(5);
        std::iota(m.begin(), m.end(), 0);

        auto pass = QubitRemapPass::Create(m);
        pass->run(qmod.get());
        ASSERT_EQ(program, qmod->toString(false));
    }

    {
        const std::string program =
"\
include \"qelib1.inc\";\
qreg q[5];\
CX q[0], q[1];\
CX q[1], q[2];\
CX q[2], q[3];\
CX q[3], q[4];\
CX q[4], q[0];\
";
        std::unique_ptr<QModule> qmod = QModule::ParseString(program);
        
        Mapping m(5);
        std::iota(m.begin(), m.end(), 0);

        auto pass = QubitRemapPass::Create(m);
        pass->run(qmod.get());
        ASSERT_EQ(program, qmod->toString(false));
    }
}

TEST(QubitRemapPassTests, SimpleRemapTest) {
    {
        const std::string program =
"\
include \"qelib1.inc\";\
qreg q[5];\
CX q[0], q[1];\
";
        const std::string result =
"\
include \"qelib1.inc\";\
qreg q[5];\
CX q[1], q[0];\
";
        std::unique_ptr<QModule> qmod = QModule::ParseString(program);
        
        Mapping m(5);
        std::iota(m.begin(), m.end(), 0);
        std::swap(m[0], m[1]);

        auto pass = QubitRemapPass::Create(m);
        pass->run(qmod.get());
        ASSERT_EQ(result, qmod->toString(false));
    }

    {
        const std::string program =
"\
include \"qelib1.inc\";\
qreg q[5];\
CX q[0], q[1];\
CX q[1], q[2];\
CX q[2], q[3];\
CX q[3], q[4];\
CX q[4], q[0];\
";
        const std::string result =
"\
include \"qelib1.inc\";\
qreg q[5];\
CX q[4], q[3];\
CX q[3], q[2];\
CX q[2], q[1];\
CX q[1], q[0];\
CX q[0], q[4];\
";
        std::unique_ptr<QModule> qmod = QModule::ParseString(program);
        
        Mapping m(5);
        std::iota(m.begin(), m.end(), 0);
        std::reverse(m.begin(), m.end());

        auto pass = QubitRemapPass::Create(m);
        pass->run(qmod.get());
        ASSERT_EQ(result, qmod->toString(false));
    }
}

TEST(QubitRemapPassTests, IfRemapTest) {
    {
        const std::string program =
"\
include \"qelib1.inc\";\
qreg q[5];\
creg c[5];\
if (c == 7) CX q[0], q[1];\
";
        const std::string result =
"\
include \"qelib1.inc\";\
qreg q[5];\
creg c[5];\
if (c == 7) CX q[1], q[0];\
";
        std::unique_ptr<QModule> qmod = QModule::ParseString(program);
        
        Mapping m(5);
        std::iota(m.begin(), m.end(), 0);
        std::swap(m[0], m[1]);

        auto pass = QubitRemapPass::Create(m);
        pass->run(qmod.get());
        ASSERT_EQ(result, qmod->toString(false));
    }

    {
        const std::string program =
"\
include \"qelib1.inc\";\
qreg q[5];\
creg c[5];\
CX q[0], q[1];\
if (c == 5) CX q[1], q[2];\
if (c == 5) CX q[2], q[3];\
if (c == 5) CX q[3], q[4];\
CX q[4], q[0];\
";
        const std::string result =
"\
include \"qelib1.inc\";\
qreg q[5];\
creg c[5];\
CX q[4], q[3];\
if (c == 5) CX q[3], q[2];\
if (c == 5) CX q[2], q[1];\
if (c == 5) CX q[1], q[0];\
CX q[0], q[4];\
";
        std::unique_ptr<QModule> qmod = QModule::ParseString(program);
        
        Mapping m(5);
        std::iota(m.begin(), m.end(), 0);
        std::reverse(m.begin(), m.end());

        auto pass = QubitRemapPass::Create(m);
        pass->run(qmod.get());
        ASSERT_EQ(result, qmod->toString(false));
    }
}
