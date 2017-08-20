
#include "gtest/gtest.h"

#include "enfield/Transform/SimpleQbitAllocator.h"
#include "enfield/Transform/IdentityMappingFinder.h"
#include "enfield/Transform/QbitterDepSolver.h"
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
5\n\
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

        auto qmod = toShared(std::move(QModule::ParseString(program)));
        auto allocator = SimpleQbitAllocator::Create(graph);
        allocator->setMapFinder(IdentityMappingFinder::Create());
        allocator->setDepSolver(QbitterDepSolver::Create());

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
CX q[2], q[1];\
CX q[2], q[0];\
CX q[1], q[0];\
CX q[4], q[3];\
cx q[2], q[0];\
cx q[4], q[2];\
cx q[2], q[0];\
cx q[4], q[2];\
cx q[2], q[0];\
cx q[3], q[2];\
cx q[2], q[0];\
cx q[3], q[2];\
";

        ArchGraph::sRef graph = getGraph();

        auto qmod = toShared(std::move(QModule::ParseString(program)));
        auto allocator = SimpleQbitAllocator::Create(graph);
        allocator->setMapFinder(IdentityMappingFinder::Create());
        allocator->setDepSolver(QbitterDepSolver::Create());

        allocator->setInlineAll({ "cx" });
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

        auto qmod = toShared(std::move(QModule::ParseString(program)));
        auto allocator = SimpleQbitAllocator::Create(graph);
        allocator->setMapFinder(IdentityMappingFinder::Create());
        allocator->setDepSolver(QbitterDepSolver::Create());

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

        auto qmod = toShared(std::move(QModule::ParseString(program)));
        auto allocator = SimpleQbitAllocator::Create(graph);
        allocator->setMapFinder(IdentityMappingFinder::Create());
        allocator->setDepSolver(QbitterDepSolver::Create());

        allocator->setInlineAll({ "cx" });
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
qreg q[5];\
CX q[0], q[1];\
CX q[0], q[2];\
CX q[1], q[2];\
cx q[2], q[1];\
cx q[4], q[2];\
cx q[2], q[1];\
cx q[4], q[2];\
cx q[2], q[0];\
cx q[4], q[2];\
cx q[2], q[0];\
cx q[4], q[2];\
CX q[1], q[0];\
";

        ArchGraph::sRef graph = getGraph();

        auto qmod = toShared(std::move(QModule::ParseString(program)));
        auto allocator = SimpleQbitAllocator::Create(graph);
        allocator->setMapFinder(IdentityMappingFinder::Create());
        allocator->setDepSolver(QbitterDepSolver::Create());

        allocator->setInlineAll({ "cx" });
        allocator->run(qmod.get());
        ASSERT_EQ(qmod->toString(), result);
    }
}
