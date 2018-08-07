
#include "gtest/gtest.h"

#include "enfield/Transform/QModuleQualityEvalPass.h"
#include "enfield/Transform/FlattenPass.h"
#include "enfield/Transform/InlineAllPass.h"
#include "enfield/Transform/Allocators/QbitAllocator.h"
#include "enfield/Transform/PassCache.h"

#include <string>

using namespace efd;

void TestForProgram(const std::string& program, QModuleQuality expected) {
    auto qmod = QModule::ParseString(program);

    auto inlinePass = InlineAllPass::Create({ "U", "CX" });
    auto qualityPass = QModuleQualityEvalPass::Create({ {"U", 1}, {"CX", 10} });
    PassCache::Run<FlattenPass>(qmod.get());
    PassCache::Run(qmod.get(), inlinePass.get());
    PassCache::Run(qmod.get(), qualityPass.get());

    auto quality = qualityPass->getData();
    EXPECT_EQ(quality.mDepth, expected.mDepth);
    EXPECT_EQ(quality.mGates, expected.mGates);
    EXPECT_EQ(quality.mWeightedCost, expected.mWeightedCost);
}

TEST(QModuleQualityEvalPassTests, ZeroCost) {
    {
        const std::string program =
"\
OPENQASM 2.0;\
include \"qelib1.inc\";\
qreg r[5];\
";
        TestForProgram(program, QModuleQuality { 0, 0, 0 });
    }
    {
        const std::string program =
"\
OPENQASM 2.0;\
include \"qelib1.inc\";\
qreg r[5];\
creg c[5];\
measure r -> c;\
";
        TestForProgram(program, QModuleQuality { 1, 5, 0 });
    }
}

TEST(QModuleQualityEvalPassTests, OneGate) {
    {
        const std::string program =
"\
OPENQASM 2.0;\
include \"qelib1.inc\";\
qreg r[5];\
U(pi, pi, pi) r[0];\
";
        TestForProgram(program, QModuleQuality { 1, 1, 1 });
    }
    {
        const std::string program =
"\
OPENQASM 2.0;\
include \"qelib1.inc\";\
qreg r[5];\
creg c[5];\
CX r[0], r[1];\
measure r -> c;\
";
        TestForProgram(program, QModuleQuality { 2, 6, 10 });
    }
}

TEST(QModuleQualityEvalPassTests, OneGenericGate) {
    {
        const std::string program =
"\
qreg q[2];\
intrinsic_rev_cx__ q[0], q[1];\
";
        TestForProgram(program, QModuleQuality { 3, 5, 14 });
    }
    {
        const std::string program =
"\
qreg q[2];\
intrinsic_swap__ q[0], q[1];\
";
        TestForProgram(program, QModuleQuality { 3, 3, 30 });
    }
    {
        const std::string program =
"\
qreg q[3];\
intrinsic_lcx__ q[0], q[1], q[2];\
";
        TestForProgram(program, QModuleQuality { 4, 4, 40 });
    }
}

TEST(QModuleQualityEvalPassTests, WholeProgram) {
    {
        const std::string program =
"\
OPENQASM 2.0;\
include \"qelib1.inc\";\
\
qreg r[5];\
\
cx r[4], r[0];\
cx r[4], r[3];\
cx r[0], r[3];\
cx r[1], r[3];\
cx r[1], r[2];\
cx r[2], r[3];\
cx r[1], r[4];\
";
        TestForProgram(program, QModuleQuality { 6, 7, 70 });
    }
    {
        const std::string program =
"\
OPENQASM 2.0;\
include \"qelib1.inc\";\
\
qreg r[5];\
\
cx r[3], r[1];\
cx r[3], r[2];\
cx r[2], r[1];\
cx r[4], r[0];\
cx r[4], r[1];\
cx r[0], r[1];\
cx r[0], r[3];\
cx r[3], r[0];\
cx r[0], r[3];\
";
        TestForProgram(program, QModuleQuality { 8, 9, 90 });
    }
    {
        const std::string program =
"\
OPENQASM 2.0;\
include \"qelib1.inc\";\
\
qreg r[5];\
\
cx r[3], r[0];\
cx r[3], r[1];\
cx r[1], r[0];\
cx r[2], r[4];\
cx r[2], r[0];\
cx r[4], r[0];\
cx r[4], r[3];\
cx r[3], r[4];\
cx r[4], r[3];\
";
        TestForProgram(program, QModuleQuality { 8, 9, 90 });
    }
    {
        const std::string program =
"\
OPENQASM 2.0;\
include \"qelib1.inc\";\
\
qreg q[5];\
\
cx q[0], q[1];\
cx q[0], q[2];\
cx q[0], q[3];\
cx q[0], q[1];\
cx q[0], q[1];\
cx q[0], q[2];\
cx q[0], q[2];\
";
        TestForProgram(program, QModuleQuality { 7, 7, 70 });
    }
    {
        const std::string program =
"\
OPENQASM 2.0;\
include \"qelib1.inc\";\
\
qreg r[5];\
\
cx r[4], r[0];\
intrinsic_rev_cx__ r[4], r[3];\
cx r[0], r[3];\
cx r[1], r[3];\
intrinsic_swap__ r[1], r[2];\
intrinsic_lcx__ r[2], r[3], r[0];\
cx r[1], r[4];\
";
        TestForProgram(program, QModuleQuality { 12, 16, 124 });
    }
    {
        const std::string program =
"\
OPENQASM 2.0;\
include \"qelib1.inc\";\
\
qreg r[5];\
\
cx r[3], r[1];\
cx r[3], r[2];\
intrinsic_rev_cx__ r[2], r[1];\
cx r[4], r[0];\
intrinsic_lcx__ r[0], r[4], r[1];\
intrinsic_swap__ r[0], r[1];\
cx r[0], r[3];\
cx r[3], r[0];\
cx r[0], r[3];\
";
        TestForProgram(program, QModuleQuality { 15, 18, 144 });
    }
    {
        const std::string program =
"\
OPENQASM 2.0;\
include \"qelib1.inc\";\
\
qreg r[5];\
\
intrinsic_swap__ r[3], r[0];\
cx r[3], r[1];\
cx r[1], r[0];\
cx r[2], r[4];\
intrinsic_lcx__ r[1], r[2], r[0];\
cx r[4], r[0];\
cx r[4], r[3];\
cx r[3], r[4];\
intrinsic_rev_cx__ r[4], r[3];\
";
        TestForProgram(program, QModuleQuality { 14, 18, 144 });
    }
    {
        const std::string program =
"\
OPENQASM 2.0;\
include \"qelib1.inc\";\
\
qreg q[5];\
\
cx q[0], q[1];\
cx q[0], q[2];\
intrinsic_lcx__ q[2], q[0], q[3];\
intrinsic_lcx__ q[2], q[0], q[1];\
cx q[0], q[1];\
intrinsic_rev_cx__ q[0], q[2];\
cx q[0], q[2];\
";
        TestForProgram(program, QModuleQuality { 15, 17, 134 });
    }
}
