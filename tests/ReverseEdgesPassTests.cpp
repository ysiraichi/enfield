
#include "gtest/gtest.h"

#include "enfield/Transform/ReverseEdgesPass.h"
#include "enfield/Arch/Architectures.h"
#include "enfield/Support/RTTI.h"
#include "enfield/Support/uRefCast.h"

#include <string>

using namespace efd;

TEST(ReverseEdgesTest, SimpleEdgeReversalTest) {
    InitializeAllArchitectures();

    {
        const std::string program = 
"\
include \"qelib1.inc\";\
qreg q[5];\
cx q[0], q[1];\
cx q[1], q[0];\
";
        const std::string result = 
"\
include \"qelib1.inc\";\
gate intrinsic_rev_cx__ a, b {h a;h b;cx b, a;h b;h a;}\
qreg q[5];\
cx q[0], q[1];\
intrinsic_rev_cx__ q[1], q[0];\
";
        auto qmod = toShared(QModule::ParseString(program));
        ArchGraph::sRef graph = efd::CreateArchitecture(Architecture::A_ibmqx2);

        auto revPass = ReverseEdgesPass::Create(graph);
        revPass->run(qmod.get());

        ASSERT_EQ(qmod->toString(), result);
    }
}

TEST(ReverseEdgesTest, ManyEdgeReversalTest) {
    {
        const std::string program = 
"\
include \"qelib1.inc\";\
qreg q[5];\
cx q[0], q[1];\
cx q[1], q[0];\
cx q[0], q[2];\
cx q[2], q[0];\
cx q[3], q[2];\
cx q[2], q[3];\
cx q[3], q[4];\
cx q[4], q[3];\
";
        const std::string result = 
"\
include \"qelib1.inc\";\
gate intrinsic_rev_cx__ a, b {h a;h b;cx b, a;h b;h a;}\
qreg q[5];\
cx q[0], q[1];\
intrinsic_rev_cx__ q[1], q[0];\
cx q[0], q[2];\
intrinsic_rev_cx__ q[2], q[0];\
cx q[3], q[2];\
intrinsic_rev_cx__ q[2], q[3];\
cx q[3], q[4];\
intrinsic_rev_cx__ q[4], q[3];\
";
        auto qmod = toShared(QModule::ParseString(program));
        ArchGraph::sRef graph = efd::CreateArchitecture(Architecture::A_ibmqx2);

        auto revPass = ReverseEdgesPass::Create(graph);
        revPass->run(qmod.get());

        ASSERT_EQ(qmod->toString(), result);
    }
}
