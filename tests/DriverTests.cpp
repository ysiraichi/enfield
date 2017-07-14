#include "gtest/gtest.h"

#include "enfield/Analysis/Driver.h"
#include "enfield/Support/RTTI.h"

#include <string>

using namespace efd;

TEST(DriverTests, DeclarationTest) {
    std::string declaration = 
"\
qreg r0[5];\
creg c0[5];\
";

    std::string declarationPrt = 
"\
qreg r0[5];\n\
creg c0[5];\n\
";
    auto root = ParseString(declaration, false);
    ASSERT_FALSE(root.get() == nullptr);
    ASSERT_EQ(root->toString(), declaration);
    ASSERT_EQ(root->toString(true), declarationPrt);
}

TEST(DriverTests, GateDeclarationTest) {
    {
        std::string gate = 
"\
gate id r0 {}\
";

        std::string gatePrt = 
"\
gate id r0 {\n}\n\
";
        auto root = ParseString(gate, false);
        ASSERT_FALSE(root.get() == nullptr);
        ASSERT_EQ(root->toString(), gate);
        ASSERT_EQ(root->toString(true), gatePrt);
    }

    {
        std::string gate = 
"\
gate id(a, b, c) r0 {}\
";

        std::string gatePrt = 
"\
gate id(a, b, c) r0 {\n}\n\
";
        auto root = ParseString(gate, false);
        ASSERT_FALSE(root.get() == nullptr);
        ASSERT_EQ(root->toString(), gate);
        ASSERT_EQ(root->toString(true), gatePrt);
    }

    {
        std::string gate = 
"\
gate id(a, b, c) r0, r1, r2, r3 {}\
";

        std::string gatePrt = 
"\
gate id(a, b, c) r0, r1, r2, r3 {\n}\n\
";
        auto root = ParseString(gate, false);
        ASSERT_FALSE(root.get() == nullptr);
        ASSERT_EQ(root->toString(), gate);
        ASSERT_EQ(root->toString(true), gatePrt);
    }
}

TEST(DriverTests, OpaqueDeclarationTest) {
    {
        std::string opaque = 
"\
opaque id r0;\
";

        std::string opaquePrt = 
"\
opaque id r0;\n\
";
        auto root = ParseString(opaque, false);
        ASSERT_FALSE(root.get() == nullptr);
        ASSERT_EQ(root->toString(), opaque);
        ASSERT_EQ(root->toString(true), opaquePrt);
    }

    {
        std::string opaque = 
"\
opaque id(a, b, c) r0;\
";

        std::string opaquePrt = 
"\
opaque id(a, b, c) r0;\n\
";
        auto root = ParseString(opaque, false);
        ASSERT_FALSE(root.get() == nullptr);
        ASSERT_EQ(root->toString(), opaque);
        ASSERT_EQ(root->toString(true), opaquePrt);
    }

    {
        std::string opaque = 
"\
opaque id(a, b, c) r0, r1, r2, r3;\
";

        std::string opaquePrt = 
"\
opaque id(a, b, c) r0, r1, r2, r3;\n\
";
        auto root = ParseString(opaque, false);
        ASSERT_FALSE(root.get() == nullptr);
        ASSERT_EQ(root->toString(), opaque);
        ASSERT_EQ(root->toString(true), opaquePrt);
    }
}

TEST(DriverTests, QOpBarrierTest) {
    {
        std::string barrier = 
"\
barrier r0;\
";

        std::string barrierPrt = 
"\
barrier r0;\n\
";
        auto root = ParseString(barrier, false);
        ASSERT_FALSE(root.get() == nullptr);
        ASSERT_EQ(root->toString(), barrier);
        ASSERT_EQ(root->toString(true), barrierPrt);
    }

    {
        std::string barrier = 
"\
barrier r0, r1, r2;\
";

        std::string barrierPrt = 
"\
barrier r0, r1, r2;\n\
";
        auto root = ParseString(barrier, false);
        ASSERT_FALSE(root.get() == nullptr);
        ASSERT_EQ(root->toString(), barrier);
        ASSERT_EQ(root->toString(true), barrierPrt);
    }
}

TEST(DriverTests, QOpResetTest) {
    {
        std::string reset = 
"\
reset r0;\
";

        std::string resetPrt = 
"\
reset r0;\n\
";
        auto root = ParseString(reset, false);
        ASSERT_FALSE(root.get() == nullptr);
        ASSERT_EQ(root->toString(), reset);
        ASSERT_EQ(root->toString(true), resetPrt);
    }

    {
        std::string reset = 
"\
reset r0[5];\
";

        std::string resetPrt = 
"\
reset r0[5];\n\
";
        auto root = ParseString(reset, false);
        ASSERT_FALSE(root.get() == nullptr);
        ASSERT_EQ(root->toString(), reset);
        ASSERT_EQ(root->toString(true), resetPrt);
    }
}

TEST(DriverTests, QOpMeasureTest) {
    {
        std::string measure = 
"\
measure r0 -> c0;\
";

        std::string measurePrt = 
"\
measure r0 -> c0;\n\
";
        auto root = ParseString(measure, false);
        ASSERT_FALSE(root.get() == nullptr);
        ASSERT_EQ(root->toString(), measure);
        ASSERT_EQ(root->toString(true), measurePrt);
    }

    {
        std::string measure = 
"\
measure r0[3] -> r5[2];\
";

        std::string measurePrt = 
"\
measure r0[3] -> r5[2];\n\
";
        auto root = ParseString(measure, false);
        ASSERT_FALSE(root.get() == nullptr);
        ASSERT_EQ(root->toString(), measure);
        ASSERT_EQ(root->toString(true), measurePrt);
    }
}

TEST(DriverTests, QOpGenericTest) {
    {
        std::string generic = 
"\
generic r0, r1;\
";

        std::string genericPrt = 
"\
generic r0, r1;\n\
";
        auto root = ParseString(generic, false);
        ASSERT_FALSE(root.get() == nullptr);
        ASSERT_EQ(root->toString(), generic);
        ASSERT_EQ(root->toString(true), genericPrt);
    }

    {
        std::string generic = 
"\
generic(3.8, pi) r2, r3[2], r5;\
";

        std::string genericPrt = 
"\
generic(3.8, pi) r2, r3[2], r5;\n\
";
        auto root = ParseString(generic, false);
        ASSERT_FALSE(root.get() == nullptr);
        ASSERT_EQ(root->toString(), generic);
        ASSERT_EQ(root->toString(true), genericPrt);
    }
}

TEST(DriverTests, IncludeTest) {
    {
        std::string generic = 
"\
include \"files/qelib1.inc\";\
";

        std::string genericPrt = 
"\
include \"files/qelib1.inc\";\n\
";
        auto root = ParseString(generic, false);
        ASSERT_FALSE(root.get() == nullptr);
        ASSERT_EQ(root->toString(), generic);
        ASSERT_EQ(root->toString(true), genericPrt);
    }
}

TEST(DriverTests, ExpTest) {
    {
        std::string exp = 
"\
generic(someid) r0;\
";
        auto root = ParseString(exp, false);
        ASSERT_FALSE(root.get() == nullptr);
        ASSERT_EQ(root->toString(), exp);
    }

    {
        std::string exp = 
"\
generic(234325) r0;\
";

        auto root = ParseString(exp, false);
        ASSERT_FALSE(root.get() == nullptr);
        ASSERT_EQ(root->toString(), exp);
    }

    {
        std::string exp = 
"\
generic(3.14159) r0;\
";

        auto root = ParseString(exp, false);
        ASSERT_FALSE(root.get() == nullptr);
        ASSERT_EQ(root->toString(), exp);
    }

    {
        std::string exp = 
"\
generic(sin((pi / 2))) r0;\
";

        auto root = ParseString(exp, false);
        ASSERT_FALSE(root.get() == nullptr);
        ASSERT_EQ(root->toString(), exp);
    }

    {
        std::string exp = 
"\
generic(sin(tan(cos(ln((pi * exp((pi / 2)))))))) r0;\
";

        auto root = ParseString(exp, false);
        ASSERT_FALSE(root.get() == nullptr);
        ASSERT_EQ(root->toString(), exp);
    }

    {
        std::string exp = 
"\
generic((((1 + 3) * 5) + (pi * sin(pi)))) r0;\
";

        auto root = ParseString(exp, false);
        ASSERT_FALSE(root.get() == nullptr);
        ASSERT_EQ(root->toString(), exp);
    }
}

TEST(DriverTests, IfStmtTest) {
    {
        std::string ifStmt = 
"\
if (pi == 3) measure r0[4] -> c2[4];\
";
        auto root = ParseString(ifStmt, false);
        ASSERT_FALSE(root.get() == nullptr);
        ASSERT_EQ(root->toString(), ifStmt);
    }
}

TEST(DriverTests, NonEmptyGateDeclTest) {
    {
        std::string gate = 
"\
gate u3(theta, phi, lambda) q {U(theta, phi, lambda) q;}\
";

        std::string gatePrt = 
"\
gate u3(theta, phi, lambda) q {\n\tU(theta, phi, lambda) q;\n}\n\
";
        auto root = ParseString(gate, false);
        ASSERT_FALSE(root.get() == nullptr);
        ASSERT_EQ(root->toString(), gate);
        ASSERT_EQ(root->toString(true), gatePrt);
    }

    {
        std::string gate = 
"\
gate rx(theta) a {u3(theta, ((-pi) / 2), (pi / 2)) a;}\
";

        std::string gatePrt = 
"\
gate rx(theta) a {\n\tu3(theta, ((-pi) / 2), (pi / 2)) a;\n}\n\
";
        auto root = ParseString(gate, false);
        ASSERT_FALSE(root.get() == nullptr);
        ASSERT_EQ(root->toString(), gate);
        ASSERT_EQ(root->toString(true), gatePrt);
    }

    {
        std::string gate = 
"\
gate ccx a, b, c {\
h c;\
cx b, c;\
tdg c;\
cx a, c;\
t c;\
cx b, c;\
tdg c;\
cx a, c;\
t b;\
t c;\
h c;\
cx a, b;\
t a;\
tdg b;\
cx a, b;\
}\
";

        std::string gatePrt = 
"\
gate ccx a, b, c {\n\
\th c;\n\
\tcx b, c;\n\
\ttdg c;\n\
\tcx a, c;\n\
\tt c;\n\
\tcx b, c;\n\
\ttdg c;\n\
\tcx a, c;\n\
\tt b;\n\
\tt c;\n\
\th c;\n\
\tcx a, b;\n\
\tt a;\n\
\ttdg b;\n\
\tcx a, b;\n\
}\n\
";
        auto root = ParseString(gate, false);
        ASSERT_FALSE(root.get() == nullptr);
        ASSERT_EQ(root->toString(), gate);
        ASSERT_EQ(root->toString(true), gatePrt);
    }

}

TEST(DriverTests, WholeProgramTest) {
    std::string program = 
"\
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
x b;\
x b[6];\
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

    std::string programPrt = 
"\
gate majority a, b, c {\n\
\tcx c, b;\n\
\tcx c, a;\n\
\tccx a, b, c;\n\
}\n\
gate unmaj a, b, c {\n\
\tccx a, b, c;\n\
\tcx c, a;\n\
\tcx a, b;\n\
}\n\
gate add4 a0, a1, a2, a3, b0, b1, b2, b3, cin, cout {\n\
\tmajority cin, b0, a0;\n\
\tmajority a0, b1, a1;\n\
\tmajority a1, b2, a2;\n\
\tmajority a2, b3, a3;\n\
\tcx a3, cout;\n\
\tunmaj a2, b3, a3;\n\
\tunmaj a1, b2, a2;\n\
\tunmaj a0, b1, a1;\n\
\tunmaj cin, b0, a0;\n\
}\n\
qreg carry[2];\n\
qreg a[8];\n\
qreg b[8];\n\
creg ans[8];\n\
creg carryout[1];\n\
x a[0];\n\
x b;\n\
x b[6];\n\
add4 a[0], a[1], a[2], a[3], b[0], b[1], b[2], b[3], carry[0], carry[1];\n\
add4 a[4], a[5], a[6], a[7], b[4], b[5], b[6], b[7], carry[1], carry[0];\n\
measure b[0] -> ans[0];\n\
measure b[1] -> ans[1];\n\
measure b[2] -> ans[2];\n\
measure b[3] -> ans[3];\n\
measure b[4] -> ans[4];\n\
measure b[5] -> ans[5];\n\
measure b[6] -> ans[6];\n\
measure b[7] -> ans[7];\n\
measure carry[0] -> carryout[0];\n\
";
    auto root = ParseString(program, false);
    ASSERT_FALSE(root.get() == nullptr);
    ASSERT_EQ(root->toString(), program);
    ASSERT_EQ(root->toString(true), programPrt);
}
