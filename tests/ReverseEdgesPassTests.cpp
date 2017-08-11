
#include "gtest/gtest.h"

#include "enfield/Transform/ReverseEdgesPass.h"
#include "enfield/Transform/DynProgQbitAllocator.h"
#include "enfield/Arch/Architectures.h"
#include "enfield/Support/RTTI.h"
#include "enfield/Support/uRefCast.h"

#include <string>

using namespace efd;

TEST(ReverseEdgesTest, SimpleEdgeReversalTest) {
    {
        const std::string program = 
"\
gate h a {}\
gate cx a, b {CX a, b;}\
qreg q[5];\
cx q[0], q[1];\
cx q[1], q[0];\
";
        const std::string result = 
"\
gate h a {}\
gate cx a, b {CX a, b;}\
qreg q[5];\
cx q[0], q[1];\
h q[0];\
h q[1];\
cx q[0], q[1];\
h q[1];\
h q[0];\
";
        auto qmod = toShared(std::move(QModule::ParseString(program, false)));
        ArchIBMQX2::sRef graph = toShared(ArchIBMQX2::Create());

        auto revPass = ReverseEdgesPass::Create(graph);
        revPass->run(qmod.get());

        ASSERT_EQ(qmod->toString(), result);
    }
}

TEST(ReverseEdgesTest, ManyEdgeReversalTest) {
    {
        const std::string program = 
"\
gate h a {}\
gate cx a, b {CX a, b;}\
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
gate h a {}\
gate cx a, b {CX a, b;}\
qreg q[5];\
cx q[0], q[1];\
h q[0];\
h q[1];\
cx q[0], q[1];\
h q[1];\
h q[0];\
cx q[0], q[2];\
h q[0];\
h q[2];\
cx q[0], q[2];\
h q[2];\
h q[0];\
cx q[3], q[2];\
h q[3];\
h q[2];\
cx q[3], q[2];\
h q[2];\
h q[3];\
cx q[3], q[4];\
h q[3];\
h q[4];\
cx q[3], q[4];\
h q[4];\
h q[3];\
";
        auto qmod = toShared(std::move(QModule::ParseString(program, false)));
        ArchIBMQX2::sRef graph = toShared(ArchIBMQX2::Create());

        auto revPass = ReverseEdgesPass::Create(graph);
        revPass->run(qmod.get());

        ASSERT_EQ(qmod->toString(), result);
    }
}
