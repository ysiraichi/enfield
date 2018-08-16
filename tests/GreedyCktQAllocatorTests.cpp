
#include "gtest/gtest.h"

#include "enfield/Transform/Allocators/GreedyCktQAllocator.h"
#include "enfield/Arch/ArchGraph.h"
#include "enfield/Support/RTTI.h"
#include "enfield/Support/uRefCast.h"

#include <string>

using namespace efd;

static ArchGraph::sRef createGraph() {
    ArchGraph::sRef g(nullptr);
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

std::string TestAllocation(const std::string program) {
    static ArchGraph::sRef g(nullptr);
    if (g.get() == nullptr) g = createGraph();

    auto qmod = QModule::ParseString(program);
    auto allocator = GreedyCktQAllocator::Create(g);
    allocator->run(qmod.get());

    return qmod->toString(false);
}

TEST(GreedyCktQAllocatorTests, SimpleNoSwapProgram) {
    {
        const std::string program =
"\
qreg q[5];\
CX q[0], q[1];\
";
        const std::string result =
"\
include \"qelib1.inc\";\
qreg q[5];\
CX q[1], q[2];\
";

        auto generatedProgram = TestAllocation(program);
        EXPECT_EQ(generatedProgram, result);
    }

    {
        const std::string program =
"\
qreg q[5];\
CX q[2], q[1];\
CX q[2], q[0];\
CX q[1], q[0];\
CX q[4], q[3];\
CX q[4], q[0];\
CX q[3], q[0];\
";
        // Expected [ 2 1 0 4 3 ]
        const std::string result =
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

        auto generatedProgram = TestAllocation(program);
        EXPECT_EQ(generatedProgram, result);
    }
}

TEST(GreedyCktQAllocatorTests, GatesTest) {
    {
        const std::string program =
"\
qreg q[5];\
gate test a, b, c {CX a, b;CX a, c;CX b, c;}\
test q[0], q[1], q[2];\
";
        const std::string result =
"\
include \"qelib1.inc\";\
qreg q[5];\
CX q[0], q[1];\
CX q[0], q[2];\
CX q[1], q[2];\
";

        auto generatedProgram = TestAllocation(program);
        EXPECT_EQ(generatedProgram, result);
    }

    {
        const std::string program =
"\
qreg q[5];\
gate test a, b, c {CX a, b;CX a, c;CX b, c;}\
test q[0], q[1], q[2];\
test q[3], q[4], q[2];\
";
        const std::string result =
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

        auto generatedProgram = TestAllocation(program);
        EXPECT_EQ(generatedProgram, result);
    }
}

TEST(GreedyCktQAllocatorTests, GatesSwapTest) {
    {
        const std::string program =
"\
qreg q[5];\
gate test a, b, c {CX a, b;CX a, c;CX b, c;}\
test q[0], q[1], q[2];\
test q[4], q[1], q[0];\
";
        // Expected mapping: [ 0 2 1 3 4 ]
        const std::string result =
"\
include \"qelib1.inc\";\
gate intrinsic_swap__ a, b {cx a, b;cx b, a;cx a, b;}\
gate intrinsic_rev_cx__ a, b {h a;h b;cx b, a;h b;h a;}\
qreg q[5];\
CX q[1], q[2];\
intrinsic_rev_cx__ q[1], q[0];\
intrinsic_rev_cx__ q[2], q[0];\
CX q[3], q[2];\
intrinsic_swap__ q[1], q[2];\
CX q[3], q[2];\
CX q[1], q[2];\
";

        auto generatedProgram = TestAllocation(program);
        EXPECT_EQ(generatedProgram, result);
    }
}
