
#include "gtest/gtest.h"

#include "enfield/Transform/DynProgQbitAllocator.h"
#include "enfield/Support/Graph.h"
#include "enfield/Support/OneRestrictionSwapFinder.h"
#include "enfield/Support/RTTI.h"

#include <string>

using namespace efd;

static std::unique_ptr<Graph> g(nullptr);

Graph* getGraph() {
    if (g.get() != nullptr) return g.get();
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

    g =  Graph::ReadString(gStr);
    return g.get();
}

TEST(DynProgQbitAllocatorTests, SimpleNoSwapProgram) {
    {
        const std::string program =
"\
qreg q[5];\
CX q[0], q[1];\
";

        Graph* graph = getGraph();

        std::unique_ptr<QModule> qmod = QModule::ParseString(program, false);
        DynProgQbitAllocator* allocator = DynProgQbitAllocator::Create(qmod.get(), graph);

        allocator->setInlineAll({ "cx" });
        allocator->run();
        ASSERT_EQ(qmod->toString(), program);
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
qreg q[5];\
CX q[0], q[1];\
CX q[0], q[2];\
CX q[1], q[2];\
CX q[3], q[4];\
CX q[3], q[2];\
CX q[4], q[2];\
";

        Graph* graph = getGraph();

        std::unique_ptr<QModule> qmod = QModule::ParseString(program, false);
        DynProgQbitAllocator* allocator = DynProgQbitAllocator::Create(qmod.get(), graph);

        allocator->setInlineAll({ "cx" });
        allocator->run();
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
qreg q[5];\
gate test a, b, c {CX a, b;CX a, c;CX b, c;}\
CX q[0], q[1];\
CX q[0], q[2];\
CX q[1], q[2];\
";

        Graph* graph = getGraph();

        std::unique_ptr<QModule> qmod = QModule::ParseString(program, false);
        DynProgQbitAllocator* allocator = DynProgQbitAllocator::Create(qmod.get(), graph);

        allocator->setInlineAll({ "cx" });
        allocator->run();
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
qreg q[5];\
gate test a, b, c {CX a, b;CX a, c;CX b, c;}\
CX q[0], q[1];\
CX q[0], q[2];\
CX q[1], q[2];\
CX q[3], q[4];\
CX q[3], q[2];\
CX q[4], q[2];\
";

        Graph* graph = getGraph();

        std::unique_ptr<QModule> qmod = QModule::ParseString(program, false);
        DynProgQbitAllocator* allocator = DynProgQbitAllocator::Create(qmod.get(), graph);

        allocator->setInlineAll({ "cx" });
        allocator->run();
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
qreg q[5];\
gate test a, b, c {CX a, b;CX a, c;CX b, c;}\
CX q[0], q[2];\
CX q[0], q[1];\
CX q[2], q[1];\
CX q[4], q[2];\
gate __swap__ a, b {cx a, b;h a;h b;cx a, b;h a;h b;cx a, b;}\
__swap__ q[0], q[2];\
CX q[4], q[2];\
CX q[0], q[2];\
";

        Graph* graph = getGraph();

        std::unique_ptr<QModule> qmod = QModule::ParseString(program, false);
        DynProgQbitAllocator* allocator = DynProgQbitAllocator::Create(qmod.get(), graph);

        allocator->setInlineAll({ "cx" });
        allocator->run();
        ASSERT_EQ(qmod->toString(), result);
    }
}
