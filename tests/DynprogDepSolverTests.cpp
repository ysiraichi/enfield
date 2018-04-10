
#include "gtest/gtest.h"

#include "enfield/Transform/Allocators/DynprogDepSolver.h"
#include "enfield/Arch/ArchGraph.h"
#include "enfield/Support/RTTI.h"
#include "enfield/Support/uRefCast.h"

#include <string>

using namespace efd;

static ArchGraph::sRef g(nullptr);

static ArchGraph::sRef getGraph() {
    if (g.get() != nullptr) return g;
    const std::string gStr =
"\
1 5\n\
q 5\n\
q[0] q[1]\n\
q[1] q[2]\n\
q[0] q[2]\n\
q[3] q[2]\n\
q[4] q[2]\n\
q[3] q[4]\n\
";

    g = toShared(ArchGraph::ReadString(gStr));
    return g;
}

TEST(DynProgQbitAllocatorTests, SimpleNoSwapProgram) {
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
        DynprogDepSolver::uRef allocator = DynprogDepSolver::Create(graph);

        allocator->setInlineAll({ "cx" });
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
        DynprogDepSolver::uRef allocator = DynprogDepSolver::Create(graph);

        allocator->setInlineAll({ "cx" });
        allocator->run(qmod.get());
        ASSERT_EQ(qmod->toString(), result);
    }
}

TEST(DynProgQbitAllocatorTests, GatesTest) {
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
        DynprogDepSolver::uRef allocator = DynprogDepSolver::Create(graph);

        allocator->setInlineAll({ "cx" });
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
        DynprogDepSolver::uRef allocator = DynprogDepSolver::Create(graph);

        allocator->setInlineAll({ "cx" });
        allocator->run(qmod.get());
        ASSERT_EQ(qmod->toString(), result);
    }
}

TEST(DynProgQbitAllocatorTests, GatesSwapTest) {
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
        DynprogDepSolver::uRef allocator = DynprogDepSolver::Create(graph);

        allocator->setInlineAll({ "cx" });
        allocator->run(qmod.get());
        ASSERT_EQ(qmod->toString(), result);
    }
}
