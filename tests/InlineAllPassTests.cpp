
#include "gtest/gtest.h"

#include "enfield/Transform/InlineAllPass.h"
#include "enfield/Transform/QModule.h"
#include "enfield/Support/uRefCast.h"

#include <string>

using namespace efd;

TEST(InlineAllPassTests, NoBasisInline) {
    {
        const std::string program =
"\
qreg q[5];\
cx q[0], q[1];\
";
        const std::string result =
"\
include \"qelib1.inc\";\
qreg q[5];\
CX q[0], q[1];\
";
        auto qmod = toShared(QModule::ParseString(program));
        auto pass = InlineAllPass::Create();
        pass->run(qmod.get());
        ASSERT_EQ(qmod->toString(), result);
    }
    {
        const std::string program =
"\
qreg q[5];\
gate somegate(a, b) x, y {\
    cx x, y;\
    cx y, x;\
    U(a, b, 10) x;\
    U(a, b, 10) y;\
}\
somegate(10, 20) q[0], q[1];\
";
        const std::string result =
"\
include \"qelib1.inc\";\
qreg q[5];\
CX q[0], q[1];\
CX q[1], q[0];\
U(10, 20, 10) q[0];\
U(10, 20, 10) q[1];\
";
        auto qmod = toShared(QModule::ParseString(program));
        auto pass = InlineAllPass::Create();
        pass->run(qmod.get());
        ASSERT_EQ(qmod->toString(), result);
    }
}

TEST(InlineAllPassTests, BasisInline) {
    {
        const std::string program =
"\
qreg q[5];\
cx q[0], q[1];\
";
        const std::string result =
"\
include \"qelib1.inc\";\
qreg q[5];\
cx q[0], q[1];\
";
        auto qmod = toShared(QModule::ParseString(program));
        auto pass = InlineAllPass::Create({ "cx" });
        pass->run(qmod.get());
        ASSERT_EQ(qmod->toString(), result);
    }
    {
        const std::string program =
"\
qreg q[5];\
gate somegate(a, b) x, y {\
    cx x, y;\
    cx y, x;\
    U(a, b, 10) x;\
    U(a, b, 10) y;\
}\
somegate(10, 20) q[0], q[1];\
";
        const std::string result =
"\
include \"qelib1.inc\";\
gate somegate(a, b) x, y {\
cx x, y;\
cx y, x;\
U(a, b, 10) x;\
U(a, b, 10) y;\
}\
qreg q[5];\
somegate(10, 20) q[0], q[1];\
";
        auto qmod = toShared(QModule::ParseString(program));
        auto pass = InlineAllPass::Create({ "somegate" });
        pass->run(qmod.get());
        ASSERT_EQ(qmod->toString(), result);
    }
}

TEST(InlineAllPassTests, IfStmtInline) {
    {
        const std::string program =
"\
qreg q[5];\
qreg c[5];\
gate mycx a, b {cx a, b;}\
measure q -> c;\
if (c == 3) mycx q[0], q[1];\
";
        const std::string result =
"\
include \"qelib1.inc\";\
qreg q[5];\
qreg c[5];\
measure q -> c;\
if (c == 3) cx q[0], q[1];\
";
        auto qmod = toShared(QModule::ParseString(program));
        auto pass = InlineAllPass::Create({ "cx" });
        pass->run(qmod.get());
        ASSERT_EQ(qmod->toString(), result);
    }
}

