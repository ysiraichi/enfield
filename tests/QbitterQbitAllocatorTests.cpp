
#include "gtest/gtest.h"

#include "enfield/Transform/Allocators/SimpleQAllocator.h"
#include "enfield/Transform/Allocators/Simple/IdentityMappingFinder.h"
#include "enfield/Transform/Allocators/Simple/QbitterSolBuilder.h"
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

TEST(QbitterQbitAllocatorTests, SimpleNoSwapProgram) {
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
        auto allocator = SimpleQAllocator::Create(graph);
        allocator->setMapFinder(IdentityMappingFinder::Create());
        allocator->setSolBuilder(QbitterSolBuilder::Create());

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
gate intrinsic_lcx__ a, c, b {cx c, b;cx a, c;cx c, b;cx a, c;}\
gate intrinsic_rev_cx__ a, b {h a;h b;cx b, a;h b;h a;}\
qreg q[5];\
intrinsic_rev_cx__ q[2], q[1];\
intrinsic_rev_cx__ q[2], q[0];\
intrinsic_rev_cx__ q[1], q[0];\
intrinsic_rev_cx__ q[4], q[3];\
intrinsic_lcx__ q[4], q[2], q[0];\
intrinsic_lcx__ q[3], q[2], q[0];\
";

        ArchGraph::sRef graph = getGraph();

        auto qmod = toShared(QModule::ParseString(program));
        auto allocator = SimpleQAllocator::Create(graph);
        allocator->setMapFinder(IdentityMappingFinder::Create());
        allocator->setSolBuilder(QbitterSolBuilder::Create());

        allocator->run(qmod.get());
        ASSERT_EQ(qmod->toString(), result);
    }
}

TEST(QbitterQbitAllocatorTests, GatesTest) {
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
        auto allocator = SimpleQAllocator::Create(graph);
        allocator->setMapFinder(IdentityMappingFinder::Create());
        allocator->setSolBuilder(QbitterSolBuilder::Create());

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
        auto allocator = SimpleQAllocator::Create(graph);
        allocator->setMapFinder(IdentityMappingFinder::Create());
        allocator->setSolBuilder(QbitterSolBuilder::Create());

        allocator->run(qmod.get());
        ASSERT_EQ(qmod->toString(), result);
    }
}

TEST(QbitterQbitAllocatorTests, GatesSwapTest) {
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
gate intrinsic_lcx__ a, c, b {cx c, b;cx a, c;cx c, b;cx a, c;}\
gate intrinsic_rev_cx__ a, b {h a;h b;cx b, a;h b;h a;}\
qreg q[5];\
CX q[0], q[1];\
CX q[0], q[2];\
CX q[1], q[2];\
intrinsic_lcx__ q[4], q[2], q[1];\
intrinsic_lcx__ q[4], q[2], q[0];\
intrinsic_rev_cx__ q[1], q[0];\
";

        ArchGraph::sRef graph = getGraph();

        auto qmod = toShared(QModule::ParseString(program));
        auto allocator = SimpleQAllocator::Create(graph);
        allocator->setMapFinder(IdentityMappingFinder::Create());
        allocator->setSolBuilder(QbitterSolBuilder::Create());

        allocator->run(qmod.get());
        ASSERT_EQ(qmod->toString(), result);
    }
}
