
#include "gtest/gtest.h"

#include "enfield/Transform/ArchVerifierPass.h"
#include "enfield/Transform/InlineAllPass.h"
#include "enfield/Transform/PassCache.h"
#include "enfield/Arch/ArchGraph.h"
#include "enfield/Support/uRefCast.h"

#include <string>

using namespace efd;

static ArchGraph::sRef getGraph() {
    static ArchGraph::sRef g(nullptr);

    if (g.get() != nullptr) return g;
    const std::string gStr =
"{\n\
    \"qubits\": 5,\n\
    \"registers\": [ {\"name\": \"q\", \"qubits\": 5} ],\n\
    \"adj\": [\n\
        [ {\"v\": \"q[1]\"}, {\"v\": \"q[2]\"} ],\n\
        [ {\"v\": \"q[2]\"} ],\n\
        [],\n\
        [ {\"v\": \"q[2]\"}, {\"v\": \"q[4]\"} ],\n\
        [ {\"v\": \"q[2]\"} ]\n\
    ]\n\
}";

    g = JsonParser<ArchGraph>::ParseString(gStr);
    return g;
}

bool ArchTest(const std::string program) {
    ArchGraph::sRef graph = getGraph();

    auto qmod = QModule::ParseString(program);
    std::vector<std::string> basis {
        "intrinsic_swap__",
        "intrinsic_rev_cx__",
        "intrinsic_lcx__",
        "cx"
    };

    auto inlpass = InlineAllPass::Create(basis);
    auto archpass = ArchVerifierPass::Create(graph);

    PassCache::Run(qmod.get(), inlpass.get());
    PassCache::Run(qmod.get(), archpass.get());

    return archpass->getData();
}

TEST(ArchVerifierPassTests, SimplePrograms) {
    {
        const std::string program =
"\
qreg q[5];\
";

        bool isValid = ArchTest(program);
        ASSERT_TRUE(isValid);
    }
    {
        const std::string program =
"\
qreg q[5];\
creg c[5];\
U((pi / 4), (pi / 4), (pi / 4)) q[0];\
U((pi / 4), (pi / 4), (pi / 4)) q[1];\
U((pi / 4), (pi / 4), (pi / 4)) q[2];\
U((pi / 4), (pi / 4), (pi / 4)) q[3];\
U((pi / 4), (pi / 4), (pi / 4)) q[4];\
";

        bool isValid = ArchTest(program);
        ASSERT_TRUE(isValid);
    }
    {
        const std::string program =
"\
qreg q[5];\
CX q[0], q[1];\
";

        bool isValid = ArchTest(program);
        ASSERT_TRUE(isValid);
    }
    {
        const std::string program =
"\
qreg q[5];\
CX q[0], q[1];\
CX q[0], q[2];\
CX q[1], q[2];\
CX q[3], q[4];\
CX q[3], q[2];\
CX q[4], q[2];\
";

        bool isValid = ArchTest(program);
        ASSERT_TRUE(isValid);
    }
}

TEST(ArchVerifierPassTests, GatesPrograms) {
    {
        const std::string program =
"\
qreg q[5];\
gate test a, b, c {CX a, b;CX a, c;CX b, c;}\
test q[0], q[1], q[2];\
";

        bool isValid = ArchTest(program);
        ASSERT_TRUE(isValid);
    }
    {
        const std::string program =
"\
qreg q[5];\
gate test a, b, c {CX a, b;CX a, c;CX b, c;}\
test q[0], q[1], q[2];\
test q[3], q[4], q[2];\
";

        bool isValid = ArchTest(program);
        ASSERT_TRUE(isValid);
    }
    {
        const std::string program =
"\
qreg q[5];\
gate test a, b, c {cx a, b;cx a, c;cx b, c;}\
test q[0], q[1], q[2];\
barrier q[0];\
barrier q[1];\
barrier q[2];\
barrier q[3];\
test q[3], q[4], q[2];\
";

        bool isValid = ArchTest(program);
        ASSERT_TRUE(isValid);
    }
    {
        const std::string program =
"\
qreg q[5];\
gate test a, b, c {cx a, b;cx a, c;cx b, c;}\
reset q[1];\
reset q[2];\
reset q[3];\
reset q[4];\
test q[0], q[1], q[2];\
test q[3], q[4], q[2];\
";

        bool isValid = ArchTest(program);
        ASSERT_TRUE(isValid);
    }
    {
        const std::string program =
"\
qreg q[5];\
gate test a, b, c {cx a, b;cx a, c;cx b, c;}\
U(pi, pi/4, pi) q[0];\
test q[0], q[1], q[2];\
test q[3], q[4], q[2];\
";

        bool isValid = ArchTest(program);
        ASSERT_TRUE(isValid);
    }
    {
        const std::string program =
"\
qreg q[5];\
gate test a, b, c {cx a, b;cx a, c;cx b, c;}\
U(pi, pi/4, pi) q[3];\
test q[0], q[1], q[2];\
test q[3], q[4], q[2];\
";

        bool isValid = ArchTest(program);
        ASSERT_TRUE(isValid);
    }
    {
        const std::string program =
"\
qreg q[5];\
creg c[5];\
gate test a, b, c {cx a, b;cx a, c;cx b, c;}\
U(pi, pi/4, pi) q[0];\
U(pi, pi/4, pi) q[3];\
U(pi, pi/4, pi) q[2];\
U(pi, pi/4, pi) q[4];\
test q[0], q[1], q[2];\
test q[3], q[4], q[2];\
";

        bool isValid = ArchTest(program);
        ASSERT_TRUE(isValid);
    }
}

TEST(ArchVerifierPassTests, IfStmtPrograms) {
    {
        const std::string program =
"\
qreg q[5];\
creg c[5];\
if (c == 10) U((pi / 4), (pi / 4), (pi / 4)) q[0];\
if (c == 10) U((pi / 4), (pi / 4), (pi / 4)) q[1];\
if (c == 10) U((pi / 4), (pi / 4), (pi / 4)) q[2];\
if (c == 10) U((pi / 4), (pi / 4), (pi / 4)) q[3];\
if (c == 10) U((pi / 4), (pi / 4), (pi / 4)) q[4];\
";

        bool isValid = ArchTest(program);
        ASSERT_TRUE(isValid);
    }
    {
        const std::string program =
"\
qreg q[5];\
creg c[5];\
if (c == 10) CX q[0], q[1];\
if (c == 10) CX q[0], q[2];\
if (c == 10) CX q[1], q[2];\
if (c == 10) CX q[3], q[4];\
if (c == 10) CX q[3], q[2];\
if (c == 10) CX q[4], q[2];\
";

        bool isValid = ArchTest(program);
        ASSERT_TRUE(isValid);
    }
}

TEST(ArchVerifierPassTests, IntrinsicGatesProgram) {
    {
        const std::string program =
"\
include \"qelib1.inc\";\
gate intrinsic_swap__ a, b {cx a, b;cx b, a;cx a, b;}\
gate intrinsic_rev_cx__ a, b {h a;h b;cx b, a;h b;h a;}\
qreg q[5];\
CX q[1], q[2];\
intrinsic_swap__ q[3], q[2];\
CX q[1], q[2];\
CX q[3], q[2];\
intrinsic_swap__ q[3], q[2];\
CX q[0], q[2];\
CX q[0], q[1];\
intrinsic_rev_cx__ q[2], q[1];\
";

        bool isValid = ArchTest(program);
        ASSERT_TRUE(isValid);
    }
    {
        const std::string program =
"\
include \"qelib1.inc\";\
gate intrinsic_rev_cx__ a, b {h a;h b;cx b, a;h b;h a;}\
gate intrinsic_lcx__ a, c, b {cx c, b;cx a, c;cx c, b;cx a, c;}\
qreg q[5];\
CX q[1], q[2];\
intrinsic_lcx__ q[1], q[2], q[3];\
intrinsic_rev_cx__ q[2], q[3];\
CX q[0], q[2];\
CX q[0], q[1];\
intrinsic_rev_cx__ q[2], q[1];\
";

        bool isValid = ArchTest(program);
        ASSERT_TRUE(isValid);
    }
}

TEST(ArchVerifierPassTests, BrokenSimplePrograms) {
    {
        const std::string program =
"\
qreg q[5];\
U((pi / 2), pi, pi) q[5];\
";

        bool isValid = ArchTest(program);
        ASSERT_FALSE(isValid);
    }
    {
        const std::string program =
"\
qreg q[5];\
cx q[5], q[3];\
";

        bool isValid = ArchTest(program);
        ASSERT_FALSE(isValid);
    }
    {
        const std::string program =
"\
qreg q[5];\
cx q[0], q[3];\
";

        bool isValid = ArchTest(program);
        ASSERT_FALSE(isValid);
    }
    {
        const std::string program =
"\
qreg q[5];\
CX q[1], q[4];\
";

        bool isValid = ArchTest(program);
        ASSERT_FALSE(isValid);
    }
    {
        const std::string program =
"\
qreg q[5];\
CX q[0], q[1];\
CX q[0], q[2];\
CX q[1], q[2];\
CX q[3], q[4];\
CX q[3], q[2];\
CX q[2], q[4];\
";

        bool isValid = ArchTest(program);
        ASSERT_FALSE(isValid);
    }
}

TEST(ArchVerifierPassTests, BrokenGatesPrograms) {
    {
        const std::string program =
"\
qreg q[5];\
gate test a, b, c {CX a, b;CX a, c;CX b, c;}\
test q[0], q[2], q[1];\
";

        bool isValid = ArchTest(program);
        ASSERT_FALSE(isValid);
    }
    {
        const std::string program =
"\
qreg q[5];\
gate test a, b, c {CX a, b;CX a, c;CX b, c;}\
test q[1], q[2], q[2];\
test q[3], q[4], q[2];\
";

        bool isValid = ArchTest(program);
        ASSERT_FALSE(isValid);
    }
    {
        const std::string program =
"\
qreg q[5];\
gate test a, b, c {cx a, b;cx a, c;cx b, c;}\
test q[0], q[1], q[2];\
barrier q[7], q[1];\
barrier q[2];\
barrier q[3];\
test q[3], q[4], q[2];\
";

        bool isValid = ArchTest(program);
        ASSERT_FALSE(isValid);
    }
    {
        const std::string program =
"\
qreg q[5];\
gate test a, b, c {cx a, b;cx a, c;cx b, c;}\
reset q[1];\
reset q[2];\
reset q[5];\
reset q[4];\
test q[0], q[1], q[2];\
test q[3], q[4], q[2];\
";

        bool isValid = ArchTest(program);
        ASSERT_FALSE(isValid);
    }
    {
        const std::string program =
"\
qreg q[5];\
gate test a, b, c {cx a, b;cx a, c;cx b, c;}\
U(pi, pi/4, pi) q[5];\
test q[0], q[1], q[2];\
test q[3], q[4], q[2];\
";

        bool isValid = ArchTest(program);
        ASSERT_FALSE(isValid);
    }
    {
        const std::string program =
"\
qreg q[5];\
gate test a, b, c {cx a, b;cx a, c;cx b, c;}\
U(pi, pi/4, pi) q[3];\
test q[0], q[1], q[3];\
test q[3], q[4], q[2];\
";

        bool isValid = ArchTest(program);
        ASSERT_FALSE(isValid);
    }
    {
        const std::string program =
"\
qreg q[5];\
creg c[5];\
gate test a, b, c {cx a, b;cx a, c;cx b, c;}\
U(pi, pi/4, pi) q[0];\
U(pi, pi/4, pi) q[3];\
U(pi, pi/4, pi) q[2];\
U(pi, pi/4, pi) q[4];\
test q[0], q[1], q[2];\
test q[3], q[4], q[1];\
";

        bool isValid = ArchTest(program);
        ASSERT_FALSE(isValid);
    }
}

TEST(ArchVerifierPassTests, BrokenIntrinsicGatesProgram) {
    {
        const std::string program =
"\
include \"qelib1.inc\";\
gate intrinsic_swap__ a, b {cx a, b;cx b, a;cx a, b;}\
gate intrinsic_rev_cx__ a, b {h a;h b;cx b, a;h b;h a;}\
qreg q[5];\
CX q[1], q[2];\
intrinsic_swap__ q[3], q[1];\
CX q[1], q[2];\
CX q[3], q[2];\
intrinsic_swap__ q[3], q[2];\
CX q[0], q[2];\
CX q[0], q[1];\
intrinsic_rev_cx__ q[2], q[1];\
";

        bool isValid = ArchTest(program);
        ASSERT_FALSE(isValid);
    }
    {
        const std::string program =
"\
include \"qelib1.inc\";\
gate intrinsic_rev_cx__ a, b {h a;h b;cx b, a;h b;h a;}\
gate intrinsic_lcx__ a, c, b {cx c, b;cx a, c;cx c, b;cx a, c;}\
qreg q[5];\
CX q[1], q[2];\
intrinsic_lcx__ q[1], q[0], q[3];\
intrinsic_rev_cx__ q[2], q[3];\
CX q[0], q[2];\
CX q[0], q[1];\
intrinsic_rev_cx__ q[2], q[1];\
";

        bool isValid = ArchTest(program);
        ASSERT_FALSE(isValid);
    }
}
