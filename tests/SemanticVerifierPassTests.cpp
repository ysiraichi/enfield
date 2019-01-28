
#include "gtest/gtest.h"

#include "enfield/Transform/SemanticVerifierPass.h"

#include <string>

using namespace efd;

bool CheckSemanticVerifier(const std::string progBefore, const std::string progAfter, Mapping map) {
    auto qmodBefore = QModule::ParseString(progBefore);
    auto qmodAfter = QModule::ParseString(progAfter);

    SemanticVerifierPass verifier(std::move(qmodBefore), map);
    verifier.run(qmodAfter.get());

    if (!verifier.getData().isSuccess()) {
        ERR << verifier.getData().getErrorMessage() << std::endl;
    }

    return verifier.getData().isSuccess();
}

TEST(SemanticVerifierPassTests, SimpleProgram) {
    {
        const std::string progBefore =
"\
qreg q[5];\
CX q[0], q[1];\
";
        const std::string progAfter =
"\
include \"qelib1.inc\";\
qreg q[5];\
CX q[1], q[2];\
";

        Mapping mapping { 1, 2, 0, 3, 4 };
        bool areSemanticalyEqual = CheckSemanticVerifier(progBefore, progAfter, mapping);

        EXPECT_TRUE(areSemanticalyEqual);
    }

    {
        const std::string progBefore =
"\
qreg q[5];\
CX q[2], q[1];\
CX q[2], q[0];\
CX q[1], q[0];\
CX q[4], q[3];\
CX q[4], q[0];\
CX q[3], q[0];\
";
        const std::string progAfter =
"\
include \"qelib1.inc\";\
qreg q[5];\
CX q[3], q[4];\
CX q[3], q[2];\
CX q[4], q[2];\
CX q[0], q[1];\
CX q[0], q[2];\
CX q[1], q[2];\
";

        Mapping mapping { 2, 4, 3, 1, 0 };
        bool areSemanticalyEqual = CheckSemanticVerifier(progBefore, progAfter, mapping);

        EXPECT_TRUE(areSemanticalyEqual);
    }
}

TEST(SemanticVerifierPassTests, BarrierProgram) {
    {
        const std::string progBefore =
"\
qreg q[5];\
barrier q[0], q[1];\
";
        const std::string progAfter =
"\
include \"qelib1.inc\";\
qreg q[5];\
barrier q[1], q[2];\
";

        Mapping mapping { 1, 2, 0, 3, 4 };
        bool areSemanticalyEqual = CheckSemanticVerifier(progBefore, progAfter, mapping);

        EXPECT_TRUE(areSemanticalyEqual);
    }

    {
        const std::string progBefore =
"\
qreg q[5];\
barrier q[2], q[1];\
barrier q[2], q[0];\
barrier q[1], q[0];\
barrier q[4], q[3];\
barrier q[4], q[0];\
barrier q[3], q[0];\
";
        const std::string progAfter =
"\
include \"qelib1.inc\";\
qreg q[5];\
barrier q[3], q[4];\
barrier q[3], q[2];\
barrier q[4], q[2];\
barrier q[0], q[1];\
barrier q[0], q[2];\
barrier q[1], q[2];\
";

        Mapping mapping { 2, 4, 3, 1, 0 };
        bool areSemanticalyEqual = CheckSemanticVerifier(progBefore, progAfter, mapping);

        EXPECT_TRUE(areSemanticalyEqual);
    }
}

TEST(SemanticVerifierPassTests, ProgramWithGatesTest) {
    {
        const std::string progBefore =
"\
qreg q[5];\
gate test a, b, c {CX a, b;CX a, c;CX b, c;}\
test q[0], q[1], q[2];\
";
        const std::string progAfter =
"\
include \"qelib1.inc\";\
qreg q[5];\
CX q[0], q[1];\
CX q[0], q[2];\
CX q[1], q[2];\
";

        Mapping mapping { 0, 1, 2, 3, 4 };
        bool areSemanticalyEqual = CheckSemanticVerifier(progBefore, progAfter, mapping);

        EXPECT_TRUE(areSemanticalyEqual);
    }
    {
        const std::string progBefore =
"\
qreg q[5];\
gate test a, b, c {CX a, b;CX a, c;CX b, c;}\
test q[0], q[1], q[2];\
test q[3], q[4], q[2];\
";
        const std::string progAfter =
"\
include \"qelib1.inc\";\
qreg q[5];\
CX q[3], q[4];\
CX q[3], q[2];\
CX q[4], q[2];\
CX q[0], q[1];\
CX q[0], q[2];\
CX q[1], q[2];\
";

        Mapping mapping { 3, 4, 2, 0, 1 };
        bool areSemanticalyEqual = CheckSemanticVerifier(progBefore, progAfter, mapping);

        EXPECT_TRUE(areSemanticalyEqual);
    }
    {
        const std::string progBefore =
"\
qreg q[5];\
gate test a, b, c {cx a, b;cx a, c;cx b, c;}\
test q[0], q[1], q[2];\
barrier q;\
test q[3], q[4], q[2];\
";
        const std::string progAfter =
"\
include \"qelib1.inc\";\
qreg q[5];\
CX q[3], q[4];\
CX q[3], q[2];\
CX q[4], q[2];\
barrier q[3];\
barrier q[4];\
barrier q[2];\
barrier q[0];\
barrier q[1];\
CX q[0], q[1];\
CX q[0], q[2];\
CX q[1], q[2];\
";

        Mapping mapping { 3, 4, 2, 0, 1 };
        bool areSemanticalyEqual = CheckSemanticVerifier(progBefore, progAfter, mapping);

        EXPECT_TRUE(areSemanticalyEqual);
    }
    {
        const std::string progBefore =
"\
qreg q[5];\
gate test a, b, c {cx a, b;cx a, c;cx b, c;}\
reset q;\
test q[0], q[1], q[2];\
test q[3], q[4], q[2];\
";
        const std::string progAfter =
"\
include \"qelib1.inc\";\
qreg q[5];\
reset q[3];\
reset q[4];\
reset q[2];\
reset q[0];\
reset q[1];\
CX q[3], q[4];\
CX q[3], q[2];\
CX q[4], q[2];\
CX q[0], q[1];\
CX q[0], q[2];\
CX q[1], q[2];\
";

        Mapping mapping { 3, 4, 2, 0, 1 };
        bool areSemanticalyEqual = CheckSemanticVerifier(progBefore, progAfter, mapping);

        EXPECT_TRUE(areSemanticalyEqual);
    }
    {
        const std::string progBefore =
"\
qreg q[5];\
gate test a, b, c {cx a, b;cx a, c;cx b, c;}\
U(pi, pi/4, pi) q[0];\
test q[0], q[1], q[2];\
test q[3], q[4], q[2];\
";
        const std::string progAfter =
"\
include \"qelib1.inc\";\
qreg q[5];\
U(pi, pi/4, pi) q[3];\
CX q[3], q[4];\
CX q[3], q[2];\
CX q[4], q[2];\
CX q[0], q[1];\
CX q[0], q[2];\
CX q[1], q[2];\
";

        Mapping mapping { 3, 4, 2, 0, 1 };
        bool areSemanticalyEqual = CheckSemanticVerifier(progBefore, progAfter, mapping);

        EXPECT_TRUE(areSemanticalyEqual);
    }
    {
        const std::string progBefore =
"\
qreg q[5];\
gate test a, b, c {cx a, b;cx a, c;cx b, c;}\
U(pi, pi/4, pi) q[3];\
test q[0], q[1], q[2];\
test q[3], q[4], q[2];\
";
        const std::string progAfter =
"\
include \"qelib1.inc\";\
qreg q[5];\
CX q[3], q[4];\
CX q[3], q[2];\
CX q[4], q[2];\
U(pi, pi/4, pi) q[0];\
CX q[0], q[1];\
CX q[0], q[2];\
CX q[1], q[2];\
";

        Mapping mapping { 3, 4, 2, 0, 1 };
        bool areSemanticalyEqual = CheckSemanticVerifier(progBefore, progAfter, mapping);

        EXPECT_TRUE(areSemanticalyEqual);
    }
   {
        const std::string progBefore =
"\
qreg q[5];\
creg c[5];\
gate test a, b, c {cx a, b;cx a, c;cx b, c;}\
U(pi, pi/4, pi) q;\
test q[0], q[1], q[2];\
test q[3], q[4], q[2];\
";
        const std::string progAfter =
"\
include \"qelib1.inc\";\
qreg q[5];\
creg c[5];\
U(pi, pi/4, pi) q[3];\
U(pi, pi/4, pi) q[4];\
U(pi, pi/4, pi) q[2];\
U(pi, pi/4, pi) q[0];\
U(pi, pi/4, pi) q[1];\
CX q[3], q[4];\
CX q[3], q[2];\
CX q[4], q[2];\
CX q[0], q[1];\
CX q[0], q[2];\
CX q[1], q[2];\
";

        Mapping mapping { 3, 4, 2, 0, 1 };
        bool areSemanticalyEqual = CheckSemanticVerifier(progBefore, progAfter, mapping);

        EXPECT_TRUE(areSemanticalyEqual);
    }
}

TEST(SemanticVerifierPassTests, IntrinsicGatesTest) {
    {
        const std::string progBefore =
"\
qreg q[5];\
gate test a, b, c {CX a, b;CX a, c;CX b, c;}\
test q[0], q[1], q[2];\
test q[4], q[1], q[0];\
";
        const std::string progAfter =
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

        Mapping mapping { 1, 2, 3, 4, 0 };
        bool areSemanticalyEqual = CheckSemanticVerifier(progBefore, progAfter, mapping);

        EXPECT_TRUE(areSemanticalyEqual);
    }
    {
        const std::string progBefore =
"\
qreg q[5];\
gate test a, b, c {CX a, b;CX a, c;CX b, c;}\
test q[0], q[1], q[2];\
test q[4], q[1], q[0];\
";
        const std::string progAfter =
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

        Mapping mapping { 1, 2, 3, 4, 0 };
        bool areSemanticalyEqual = CheckSemanticVerifier(progBefore, progAfter, mapping);

        EXPECT_TRUE(areSemanticalyEqual);
    }
}

TEST(SemanticVerifierPassTests, IfStmtsTest) {
    {
        const std::string progBefore =
"\
qreg q[5];\
creg c[5];\
gate test(x) a, b, c {U(x, x, x) a;U(x, x, x) b;U(x, x, x) c;}\
test(pi/4) q[0], q[1], q[2];\
measure q -> c;\
if (c == 4) test(pi/2) q[4], q[1], q[0];\
";
        const std::string progAfter =
"\
include \"qelib1.inc\";\
qreg q[5];\
creg c[5];\
measure q[4] -> c[3];\
measure q[0] -> c[4];\
U((pi / 4), (pi / 4), (pi / 4)) q[1];\
measure q[1] -> c[0];\
U((pi / 4), (pi / 4), (pi / 4)) q[2];\
measure q[2] -> c[1];\
U((pi / 4), (pi / 4), (pi / 4)) q[3];\
measure q[3] -> c[2];\
if (c == 4) U((pi / 2), (pi / 2), (pi / 2)) q[0];\
if (c == 4) U((pi / 2), (pi / 2), (pi / 2)) q[2];\
if (c == 4) U((pi / 2), (pi / 2), (pi / 2)) q[1];\
";

        Mapping mapping { 1, 2, 3, 4, 0 };
        bool areSemanticalyEqual = CheckSemanticVerifier(progBefore, progAfter, mapping);

        EXPECT_TRUE(areSemanticalyEqual);
    }
    {
        const std::string progBefore =
"\
qreg q[5];\
creg c[5];\
gate test a, b, c {CX a, b;CX a, c;CX b, c;}\
test q[0], q[1], q[2];\
measure q -> c;\
if (c == 4) test q[4], q[1], q[0];\
";
        const std::string progAfter =
"\
include \"qelib1.inc\";\
gate intrinsic_swap__ a, b {cx a, b;cx b, a;cx a, b;}\
gate intrinsic_rev_cx__ a, b {h a;h b;cx b, a;h b;h a;}\
qreg q[5];\
creg c[5];\
measure q[4] -> c[3];\
measure q[0] -> c[4];\
CX q[1], q[2];\
intrinsic_swap__ q[3], q[2];\
CX q[1], q[2];\
measure q[1] -> c[0];\
CX q[3], q[2];\
measure q[3] -> c[1];\
measure q[2] -> c[2];\
intrinsic_swap__ q[3], q[2];\
if (c == 4) CX q[0], q[2];\
if (c == 4) CX q[0], q[1];\
if (c == 4) intrinsic_rev_cx__ q[2], q[1];\
";

        Mapping mapping { 1, 2, 3, 4, 0 };
        bool areSemanticalyEqual = CheckSemanticVerifier(progBefore, progAfter, mapping);

        EXPECT_TRUE(areSemanticalyEqual);
    }
}

TEST(SemanticVerifierPassTests, BrokenDependencyTest) {
    {
        // Swap comes before expected.
        const std::string progBefore =
"\
qreg q[5];\
gate test a, b, c {CX a, b;CX a, c;CX b, c;}\
test q[0], q[1], q[2];\
test q[4], q[1], q[0];\
";
        const std::string progAfter =
"\
include \"qelib1.inc\";\
gate intrinsic_swap__ a, b {cx a, b;cx b, a;cx a, b;}\
gate intrinsic_rev_cx__ a, b {h a;h b;cx b, a;h b;h a;}\
qreg q[5];\
CX q[1], q[2];\
CX q[1], q[2];\
intrinsic_swap__ q[3], q[2];\
CX q[3], q[2];\
intrinsic_swap__ q[3], q[2];\
CX q[0], q[2];\
CX q[0], q[1];\
intrinsic_rev_cx__ q[2], q[1];\
";

        Mapping mapping { 1, 2, 3, 4, 0 };
        bool areSemanticalyEqual = CheckSemanticVerifier(progBefore, progAfter, mapping);

        EXPECT_FALSE(areSemanticalyEqual);
    }
    {
        // Dependency comes after the dependent.
        const std::string progBefore =
"\
qreg q[5];\
gate test a, b, c {CX a, b;CX a, c;CX b, c;}\
test q[0], q[1], q[2];\
test q[4], q[1], q[0];\
";
        const std::string progAfter =
"\
include \"qelib1.inc\";\
gate intrinsic_swap__ a, b {cx a, b;cx b, a;cx a, b;}\
gate intrinsic_rev_cx__ a, b {h a;h b;cx b, a;h b;h a;}\
qreg q[5];\
CX q[1], q[2];\
intrinsic_swap__ q[3], q[2];\
CX q[3], q[2];\
CX q[1], q[2];\
intrinsic_swap__ q[3], q[2];\
CX q[0], q[2];\
CX q[0], q[1];\
intrinsic_rev_cx__ q[2], q[1];\
";

        Mapping mapping { 1, 2, 3, 4, 0 };
        bool areSemanticalyEqual = CheckSemanticVerifier(progBefore, progAfter, mapping);

        EXPECT_FALSE(areSemanticalyEqual);
    }
    {
        // Intrinsics swapped.
        const std::string progBefore =
"\
qreg q[5];\
gate test a, b, c {CX a, b;CX a, c;CX b, c;}\
test q[0], q[1], q[2];\
test q[4], q[1], q[0];\
";
        const std::string progAfter =
"\
include \"qelib1.inc\";\
gate intrinsic_rev_cx__ a, b {h a;h b;cx b, a;h b;h a;}\
gate intrinsic_lcx__ a, c, b {cx c, b;cx a, c;cx c, b;cx a, c;}\
qreg q[5];\
CX q[1], q[2];\
intrinsic_rev_cx__ q[2], q[3];\
intrinsic_lcx__ q[1], q[2], q[3];\
CX q[0], q[2];\
CX q[0], q[1];\
intrinsic_rev_cx__ q[2], q[1];\
";

        Mapping mapping { 1, 2, 3, 4, 0 };
        bool areSemanticalyEqual = CheckSemanticVerifier(progBefore, progAfter, mapping);

        EXPECT_FALSE(areSemanticalyEqual);
    }
    {
        // Intrinsics swapped.
        const std::string progBefore =
"\
qreg q[5];\
gate test a, b, c {CX a, b;CX a, c;CX b, c;}\
test q[0], q[1], q[2];\
test q[4], q[1], q[0];\
";
        const std::string progAfter =
"\
include \"qelib1.inc\";\
qreg q[5];\
cx q[0], q[1];\
cx q[0], q[1];\
cx q[1], q[2];\
cx q[4], q[1];\
cx q[4], q[0];\
";

        Mapping mapping { 1, 2, 3, 4, 0 };
        bool areSemanticalyEqual = CheckSemanticVerifier(progBefore, progAfter, mapping);

        EXPECT_FALSE(areSemanticalyEqual);
    }
    {
        // Intrinsics swapped.
        const std::string progBefore =
"\
qreg q[5];\
gate test a, b, c {CX a, b;CX a, c;CX b, c;}\
test q[0], q[1], q[2];\
test q[4], q[1], q[0];\
";
        const std::string progAfter =
"\
include \"qelib1.inc\";\
qreg q[5];\
";

        Mapping mapping { 1, 2, 3, 4, 0 };
        bool areSemanticalyEqual = CheckSemanticVerifier(progBefore, progAfter, mapping);

        EXPECT_FALSE(areSemanticalyEqual);
    }
}

TEST(SemanticVerifierPassTests, BrokenIfStmtsTest) {
    {
        const std::string progBefore =
"\
qreg q[5];\
creg c[5];\
gate test(x) a, b, c {U(x, x, x) a;U(x, x, x) b;U(x, x, x) c;}\
test(pi/4) q[0], q[1], q[2];\
measure q -> c;\
if (c == 4) test(pi/2) q[4], q[1], q[0];\
";
        const std::string progAfter =
"\
include \"qelib1.inc\";\
qreg q[5];\
creg c[5];\
U((pi / 4), (pi / 4), (pi / 4)) q[1];\
measure q[1] -> c[0];\
U((pi / 4), (pi / 4), (pi / 4)) q[2];\
measure q[2] -> c[1];\
U((pi / 4), (pi / 4), (pi / 4)) q[3];\
measure q[3] -> c[2];\
if (c == 4) U((pi / 2), (pi / 2), (pi / 2)) q[0];\
measure q[4] -> c[3];\
measure q[0] -> c[4];\
if (c == 4) U((pi / 2), (pi / 2), (pi / 2)) q[2];\
if (c == 4) U((pi / 2), (pi / 2), (pi / 2)) q[1];\
";

        Mapping mapping { 1, 2, 3, 4, 0 };
        bool areSemanticalyEqual = CheckSemanticVerifier(progBefore, progAfter, mapping);

        EXPECT_FALSE(areSemanticalyEqual);
    }
    {
        const std::string progBefore =
"\
qreg q[5];\
creg c[5];\
gate test a, b, c {CX a, b;CX a, c;CX b, c;}\
test q[0], q[1], q[2];\
measure q -> c;\
if (c == 4) test q[4], q[1], q[0];\
";
        const std::string progAfter =
"\
include \"qelib1.inc\";\
gate intrinsic_swap__ a, b {cx a, b;cx b, a;cx a, b;}\
gate intrinsic_rev_cx__ a, b {h a;h b;cx b, a;h b;h a;}\
qreg q[5];\
creg c[5];\
measure q[3] -> c[3];\
measure q[4] -> c[4];\
CX q[1], q[2];\
intrinsic_swap__ q[3], q[2];\
CX q[1], q[2];\
measure q[0] -> c[0];\
CX q[3], q[2];\
measure q[1] -> c[1];\
measure q[2] -> c[2];\
intrinsic_swap__ q[3], q[2];\
if (c == 4) CX q[0], q[2];\
if (c == 4) CX q[0], q[1];\
if (c == 4) intrinsic_rev_cx__ q[2], q[1];\
";

        Mapping mapping { 1, 2, 3, 4, 0 };
        bool areSemanticalyEqual = CheckSemanticVerifier(progBefore, progAfter, mapping);

        EXPECT_FALSE(areSemanticalyEqual);
    }
    {
        const std::string progBefore =
"\
qreg q[5];\
creg c[5];\
gate test a, b, c {CX a, b;CX a, c;CX b, c;}\
test q[0], q[1], q[2];\
measure q -> c;\
if (c == 4) test q[4], q[1], q[0];\
";
        const std::string progAfter =
"\
include \"qelib1.inc\";\
gate intrinsic_swap__ a, b {cx a, b;cx b, a;cx a, b;}\
gate intrinsic_rev_cx__ a, b {h a;h b;cx b, a;h b;h a;}\
qreg q[5];\
creg c[5];\
measure q[4] -> c[3];\
measure q[0] -> c[4];\
CX q[1], q[2];\
intrinsic_swap__ q[3], q[2];\
CX q[1], q[2];\
measure q[1] -> c[0];\
measure q[3] -> c[1];\
CX q[3], q[2];\
measure q[2] -> c[2];\
intrinsic_swap__ q[3], q[2];\
if (c == 4) CX q[0], q[2];\
if (c == 4) CX q[0], q[1];\
if (c == 4) intrinsic_rev_cx__ q[2], q[1];\
";

        Mapping mapping { 1, 2, 3, 4, 0 };
        bool areSemanticalyEqual = CheckSemanticVerifier(progBefore, progAfter, mapping);

        EXPECT_FALSE(areSemanticalyEqual);
    }
    {
        const std::string progBefore =
"\
qreg q[5];\
creg c[5];\
gate test a, b, c {CX a, b;CX a, c;CX b, c;}\
test q[0], q[1], q[2];\
measure q -> c;\
if (c == 4) test q[4], q[1], q[0];\
";
        const std::string progAfter =
"\
include \"qelib1.inc\";\
gate intrinsic_swap__ a, b {cx a, b;cx b, a;cx a, b;}\
gate intrinsic_rev_cx__ a, b {h a;h b;cx b, a;h b;h a;}\
qreg q[5];\
creg c[5];\
measure q[4] -> c[3];\
measure q[0] -> c[4];\
CX q[1], q[2];\
intrinsic_swap__ q[3], q[2];\
CX q[1], q[2];\
measure q[1] -> c[0];\
measure q[2] -> c[2];\
CX q[3], q[2];\
measure q[3] -> c[1];\
intrinsic_swap__ q[3], q[2];\
if (c == 4) CX q[0], q[2];\
if (c == 4) CX q[0], q[1];\
if (c == 4) intrinsic_rev_cx__ q[2], q[1];\
";

        Mapping mapping { 1, 2, 3, 4, 0 };
        bool areSemanticalyEqual = CheckSemanticVerifier(progBefore, progAfter, mapping);

        EXPECT_FALSE(areSemanticalyEqual);
    }
    {
        const std::string progBefore =
"\
qreg q[5];\
creg c[5];\
gate test a, b, c {CX a, b;CX a, c;CX b, c;}\
test q[0], q[1], q[2];\
measure q -> c;\
if (c == 4) test q[4], q[1], q[0];\
";
        const std::string progAfter =
"\
include \"qelib1.inc\";\
gate intrinsic_swap__ a, b {cx a, b;cx b, a;cx a, b;}\
gate intrinsic_rev_cx__ a, b {h a;h b;cx b, a;h b;h a;}\
qreg q[5];\
creg c[5];\
measure q[4] -> c[3];\
measure q[0] -> c[4];\
CX q[1], q[2];\
intrinsic_swap__ q[3], q[2];\
CX q[1], q[2];\
measure q[1] -> c[0];\
CX q[3], q[2];\
measure q[3] -> c[1];\
measure q[2] -> c[2];\
intrinsic_swap__ q[3], q[2];\
CX q[0], q[2];\
CX q[0], q[1];\
intrinsic_rev_cx__ q[2], q[1];\
";

        Mapping mapping { 1, 2, 3, 4, 0 };
        bool areSemanticalyEqual = CheckSemanticVerifier(progBefore, progAfter, mapping);

        EXPECT_FALSE(areSemanticalyEqual);
    }
}

TEST(SemanticVerifierPassTests, BrokenCRegUseTest) {
    {
        const std::string progBefore =
"\
qreg q[5];\
creg c1[5];\
creg c2[3];\
gate test(x) a, b, c {U(x, x, x) a;U(x, x, x) b;U(x, x, x) c;}\
test(pi/4) q[0], q[1], q[2];\
measure q -> c1;\
if (c1 == 4) test(pi/2) q[4], q[1], q[0];\
";
        const std::string progAfter =
"\
include \"qelib1.inc\";\
qreg q[5];\
creg c1[5];\
creg c2[3];\
measure q[4] -> c1[3];\
measure q[0] -> c1[4];\
U((pi / 4), (pi / 4), (pi / 4)) q[1];\
measure q[1] -> c1[0];\
U((pi / 4), (pi / 4), (pi / 4)) q[2];\
measure q[2] -> c1[1];\
U((pi / 4), (pi / 4), (pi / 4)) q[3];\
measure q[3] -> c1[2];\
if (c2 == 4) U((pi / 2), (pi / 2), (pi / 2)) q[0];\
if (c2 == 4) U((pi / 2), (pi / 2), (pi / 2)) q[2];\
if (c2 == 4) U((pi / 2), (pi / 2), (pi / 2)) q[1];\
";

        Mapping mapping { 1, 2, 3, 4, 0 };
        bool areSemanticalyEqual = CheckSemanticVerifier(progBefore, progAfter, mapping);

        EXPECT_FALSE(areSemanticalyEqual);
    }
}

TEST(SemanticVerifierPassTests, WPMOrd7x1mod15Test) {
    {
        const std::string progBefore =
"\
OPENQASM 2.0;\
include \"qelib1.inc\";\
qreg q[5];\
creg c[5];\
x q[4];\
x q[1];\
x q[2];\
x q[3];\
x q[4];\
cx q[3],q[2];\
cx q[2],q[3];\
cx q[3],q[2];\
cx q[2],q[1];\
cx q[1],q[2];\
cx q[2],q[1];\
cx q[4],q[1];\
cx q[1],q[4];\
cx q[4],q[1];\
measure q[1] -> c[1];\
measure q[2] -> c[2];\
measure q[3] -> c[3];\
measure q[4] -> c[4];\
";
        const std::string progAfter =
"\
include \"qelib1.inc\";\
gate intrinsic_swap__ a, b {cx a, b; cx b, a;cx a, b;}\
gate intrinsic_rev_cx__ a, b {h a;h b;cx b, a;h b;h a;}\
creg c[5];\
qreg q[5];\
U(pi, 0, pi) q[3];\
U(pi, 0, pi) q[1];\
U(pi, 0, pi) q[0];\
U(pi, 0, pi) q[2];\
U(pi, 0, pi) q[3];\
intrinsic_rev_cx__ q[2], q[0];\
CX q[0], q[2];\
intrinsic_rev_cx__ q[2], q[0];\
CX q[0], q[1];\
intrinsic_rev_cx__ q[1], q[0];\
CX q[0], q[1];\
intrinsic_swap__ q[1], q[2];\
CX q[3], q[2];\
intrinsic_rev_cx__ q[2], q[3];\
CX q[3], q[2];\
measure q[2] -> c[1];\
measure q[0] -> c[2];\
measure q[1] -> c[3];\
measure q[3] -> c[4];\
";

        Mapping mapping { 4, 1, 0, 2, 3 };
        bool areSemanticalyEqual = CheckSemanticVerifier(progBefore, progAfter, mapping);

        EXPECT_TRUE(areSemanticalyEqual);
    }
    {
        const std::string progBefore =
"\
OPENQASM 2.0;\
include \"qelib1.inc\";\
qreg q[5];\
creg c[5];\
x q[4];\
x q[1];\
x q[2];\
x q[3];\
x q[4];\
cx q[3],q[2];\
cx q[2],q[3];\
cx q[3],q[2];\
cx q[2],q[1];\
cx q[1],q[2];\
cx q[2],q[1];\
cx q[4],q[1];\
cx q[1],q[4];\
cx q[4],q[1];\
measure q[1] -> c[1];\
measure q[2] -> c[2];\
measure q[3] -> c[3];\
measure q[4] -> c[4];\
";
        const std::string progAfter =
"\
include \"qelib1.inc\";\
gate intrinsic_rev_cx__ a, b {h a;h b;cx b, a;h b;h a;}\
creg c[5];\
qreg q[5];\
U(pi, 0, pi) q[1];\
U(pi, 0, pi) q[2];\
U(pi, 0, pi) q[4];\
U(pi, 0, pi) q[3];\
U(pi, 0, pi) q[1];\
cx q[3], q[4];\
intrinsic_rev_cx__ q[4], q[3];\
cx q[3], q[4];\
cx q[4], q[2];\
intrinsic_rev_cx__ q[2], q[4];\
cx q[4], q[2];\
cx q[1], q[2];\
intrinsic_rev_cx__ q[2], q[1];\
cx q[1], q[2];\
measure q[2] -> c[1];\
measure q[4] -> c[2];\
measure q[3] -> c[3];\
measure q[1] -> c[4];\
";

        Mapping mapping { 0, 2, 4, 3, 1 };
        bool areSemanticalyEqual = CheckSemanticVerifier(progBefore, progAfter, mapping);

        EXPECT_TRUE(areSemanticalyEqual);
    }
    {
        const std::string progBefore =
"\
OPENQASM 2.0;\
include \"qelib1.inc\";\
qreg q[5];\
creg c[5];\
x q[4];\
x q[1];\
x q[2];\
x q[3];\
x q[4];\
cx q[3],q[2];\
cx q[2],q[3];\
cx q[3],q[2];\
cx q[2],q[1];\
cx q[1],q[2];\
cx q[2],q[1];\
cx q[4],q[1];\
cx q[1],q[4];\
cx q[4],q[1];\
measure q[1] -> c[1];\
measure q[2] -> c[2];\
measure q[3] -> c[3];\
measure q[4] -> c[4];\
";
        const std::string progAfter =
"\
include \"qelib1.inc\";\
gate intrinsic_lcx__ a, c, b {cx c, b;cx a, c;cx c, b;cx a, c;}\
gate intrinsic_rev_cx__ a, b {h a;h b;cx b, a;h b;h a;}\
creg c[5];\
qreg q[5];\
U(pi, 0, pi) q[4];\
U(pi, 0, pi) q[1];\
U(pi, 0, pi) q[2];\
U(pi, 0, pi) q[3];\
U(pi, 0, pi) q[4];\
cx q[3], q[2];\
intrinsic_rev_cx__ q[2], q[3];\
cx q[3], q[2];\
intrinsic_rev_cx__ q[2], q[1];\
cx q[1], q[2];\
intrinsic_rev_cx__ q[2], q[1];\
intrinsic_lcx__ q[4], q[2], q[1];\
intrinsic_lcx__ q[1], q[2], q[4];\
intrinsic_lcx__ q[4], q[2], q[1];\
measure q[1] -> c[1];\
measure q[2] -> c[2];\
measure q[3] -> c[3];\
measure q[4] -> c[4];\
";

        Mapping mapping { 0, 1, 2, 3, 4 };
        bool areSemanticalyEqual = CheckSemanticVerifier(progBefore, progAfter, mapping);

        EXPECT_TRUE(areSemanticalyEqual);
    }
}
