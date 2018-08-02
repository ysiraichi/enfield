
#include "gtest/gtest.h"

#include "enfield/Transform/IntrinsicGateCostPass.h"
#include "enfield/Transform/Allocators/QbitAllocator.h"
#include "enfield/Transform/PassCache.h"

#include <string>

using namespace efd;

void TestBuiltGraph(const std::string& program, uint32_t expectedCost) {
    auto qmod = QModule::ParseString(program);
    uint32_t cost = PassCache::Get<IntrinsicGateCostPass>(qmod.get())->getData();
    EXPECT_EQ(cost, expectedCost);
}

TEST(IntrinsicGateCostPassTests, NoIntrinsicGate) {
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
        TestBuiltGraph(program, 0);
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
        TestBuiltGraph(program, 0);
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
        TestBuiltGraph(program, 0);
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
        TestBuiltGraph(program, 0);
    }
}

TEST(IntrinsicGateCostPassTests, SingleIntrinsicGate) {
    {
        const std::string program =
"\
qreg q[2];\
intrinsic_rev_cx__ q[0], q[1];\
";
        TestBuiltGraph(program, RevCost.getVal());
    }
    {
        const std::string program =
"\
qreg q[2];\
intrinsic_swap__ q[0], q[1];\
";
        TestBuiltGraph(program, SwapCost.getVal());
    }
    {
        const std::string program =
"\
qreg q[2];\
intrinsic_lcx__ q[0], q[1], q[2];\
";
        TestBuiltGraph(program, LCXCost.getVal());
    }
}

TEST(IntrinsicGateCostPassTests, WholeProgram) {
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
        TestBuiltGraph(program, RevCost.getVal() + SwapCost.getVal() + LCXCost.getVal());
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
        TestBuiltGraph(program, RevCost.getVal() + SwapCost.getVal() + LCXCost.getVal());
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
        TestBuiltGraph(program, RevCost.getVal() + SwapCost.getVal() + LCXCost.getVal());
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
        TestBuiltGraph(program, (2 * LCXCost.getVal()) + RevCost.getVal());
    }
}
