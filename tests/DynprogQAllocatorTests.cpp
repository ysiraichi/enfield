
#include "gtest/gtest.h"

#include "enfield/Transform/Allocators/DynprogQAllocator.h"
#include "enfield/Arch/ArchGraph.h"
#include "enfield/Support/RTTI.h"
#include "enfield/Support/uRefCast.h"

#include <string>

using namespace efd;

static ArchGraph::sRef g(nullptr);

static ArchGraph::sRef getGraph() {
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

TEST(DynprogQAllocatorTests, SimpleNoSwapProgram) {
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
CX q[0], q[1];\
";

        ArchGraph::sRef graph = getGraph();

        auto qmod = toShared(QModule::ParseString(program));
        DynprogQAllocator::uRef allocator = DynprogQAllocator::Create(graph);

        allocator->run(qmod.get());
        ASSERT_EQ(qmod->toString(), result);
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
CX q[0], q[1];\
CX q[0], q[2];\
CX q[1], q[2];\
CX q[3], q[4];\
CX q[3], q[2];\
CX q[4], q[2];\
";

        ArchGraph::sRef graph = getGraph();

        auto qmod = toShared(QModule::ParseString(program));
        DynprogQAllocator::uRef allocator = DynprogQAllocator::Create(graph);

        allocator->run(qmod.get());
        ASSERT_EQ(qmod->toString(), result);
    }
}

TEST(DynprogQAllocatorTests, GatesTest) {
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

        ArchGraph::sRef graph = getGraph();

        auto qmod = toShared(QModule::ParseString(program));
        DynprogQAllocator::uRef allocator = DynprogQAllocator::Create(graph);

        allocator->run(qmod.get());
        ASSERT_EQ(qmod->toString(), result);
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
CX q[0], q[1];\
CX q[0], q[2];\
CX q[1], q[2];\
CX q[3], q[4];\
CX q[3], q[2];\
CX q[4], q[2];\
";

        ArchGraph::sRef graph = getGraph();

        auto qmod = toShared(QModule::ParseString(program));
        DynprogQAllocator::uRef allocator = DynprogQAllocator::Create(graph);

        allocator->run(qmod.get());
        ASSERT_EQ(qmod->toString(), result);
    }
}

TEST(DynprogQAllocatorTests, GatesSwapTest) {
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
CX q[0], q[2];\
CX q[0], q[1];\
intrinsic_rev_cx__ q[2], q[1];\
CX q[4], q[2];\
intrinsic_swap__ q[0], q[2];\
CX q[4], q[2];\
CX q[0], q[2];\
";

        ArchGraph::sRef graph = getGraph();

        auto qmod = toShared(QModule::ParseString(program));
        DynprogQAllocator::uRef allocator = DynprogQAllocator::Create(graph);

        allocator->run(qmod.get());
        ASSERT_EQ(qmod->toString(), result);
    }
}
