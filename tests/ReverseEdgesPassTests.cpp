
#include "gtest/gtest.h"

#include "enfield/Transform/ReverseEdgesPass.h"
#include "enfield/Transform/DynProgQbitAllocator.h"
#include "enfield/Support/OneRestrictionSwapFinder.h"
#include "enfield/Arch/Architectures.h"
#include "enfield/Support/RTTI.h"

#include <string>

using namespace efd;

TEST(ReverseEdgesPassTests, SimpleNoSwapProgram) {
    {
        const std::string program =
"\
qreg q[5];\
gate h a {}\
CX q[0], q[1];\
";

        Graph* graph = ArchIBMQX2::Create().release();

        std::unique_ptr<QModule> qmod = QModule::ParseString(program, false);
        DependencyBuilderPass* depPass = DependencyBuilderPass::Create(qmod.get());
        DynProgQbitAllocator* allocator = DynProgQbitAllocator::Create
            (qmod.get(), graph, OneRestrictionSwapFinder::Create(graph), depPass);

        allocator->run();
        ASSERT_EQ(qmod->toString(), program);

        ReverseEdgesPass* revPass = ReverseEdgesPass::Create(qmod.get(), graph, depPass);
        qmod->runPass(revPass);
        ASSERT_EQ(qmod->toString(), program);
    }

    {
        const std::string program =
"\
gate h a {}\
qreg q[5];\
CX q[2], q[1];\
CX q[2], q[0];\
CX q[1], q[0];\
CX q[4], q[3];\
CX q[4], q[0];\
CX q[3], q[0];\
";
        const std::string revResult =
"\
qreg q[5];\
gate h a {}\
CX q[0], q[1];\
CX q[0], q[2];\
CX q[1], q[2];\
CX q[3], q[4];\
CX q[3], q[2];\
CX q[4], q[2];\
";

        Graph* graph = ArchIBMQX2::Create().release();

        std::unique_ptr<QModule> qmod = QModule::ParseString(program, false);
        DependencyBuilderPass* depPass = DependencyBuilderPass::Create(qmod.get());
        DynProgQbitAllocator* allocator = DynProgQbitAllocator::Create
            (qmod.get(), graph, OneRestrictionSwapFinder::Create(graph), depPass);

        allocator->run();

        ReverseEdgesPass* revPass = ReverseEdgesPass::Create(qmod.get(), graph, depPass);
        qmod->runPass(revPass);
        ASSERT_EQ(qmod->toString(), revResult);
    }
}

TEST(ReverseEdgesPassTests, GatesTest) {
    {
        const std::string program =
"\
gate h a {}\
qreg q[5];\
gate test a, b, c {CX a, b;CX a, c;CX b, c;}\
test q[0], q[1], q[2];\
";
        const std::string result =
"\
qreg q[5];\
gate h a {}\
gate test a, b, c {CX a, b;CX a, c;CX b, c;}\
CX q[0], q[1];\
CX q[0], q[2];\
CX q[1], q[2];\
";

        Graph* graph = ArchIBMQX2::Create().release();

        std::unique_ptr<QModule> qmod = QModule::ParseString(program, false);
        DependencyBuilderPass* depPass = DependencyBuilderPass::Create(qmod.get());
        DynProgQbitAllocator* allocator = DynProgQbitAllocator::Create
            (qmod.get(), graph, OneRestrictionSwapFinder::Create(graph), depPass);

        allocator->run();

        ReverseEdgesPass* revPass = ReverseEdgesPass::Create(qmod.get(), graph, depPass);
        qmod->runPass(revPass);
        ASSERT_EQ(qmod->toString(), result);
    }

    {
        const std::string program =
"\
gate h a {}\
qreg q[5];\
gate test a, b, c {CX a, b;CX a, c;CX b, c;}\
test q[0], q[1], q[2];\
test q[3], q[4], q[2];\
";
        const std::string result =
"\
qreg q[5];\
gate h a {}\
gate test a, b, c {CX a, b;CX a, c;CX b, c;}\
CX q[0], q[1];\
CX q[0], q[2];\
CX q[1], q[2];\
CX q[3], q[4];\
CX q[3], q[2];\
CX q[4], q[2];\
";

        Graph* graph = ArchIBMQX2::Create().release();

        std::unique_ptr<QModule> qmod = QModule::ParseString(program, false);
        DependencyBuilderPass* depPass = DependencyBuilderPass::Create(qmod.get());
        DynProgQbitAllocator* allocator = DynProgQbitAllocator::Create
            (qmod.get(), graph, OneRestrictionSwapFinder::Create(graph), depPass);

        allocator->run();

        ReverseEdgesPass* revPass = ReverseEdgesPass::Create(qmod.get(), graph, depPass);
        qmod->runPass(revPass);
        ASSERT_EQ(qmod->toString(), result);
    }
}

TEST(ReverseEdgesPassTests, GatesSwapTest) {
    {
        const std::string program =
"\
gate h a {}\
qreg q[5];\
gate test a, b, c {CX a, b;CX a, c;CX b, c;}\
test q[0], q[1], q[2];\
test q[4], q[1], q[0];\
";
        const std::string result =
"\
gate __swap__ a, b {cx a, b;h a;h b;cx a, b;h a;h b;cx a, b;}\
qreg q[5];\
gate h a {}\
gate test a, b, c {CX a, b;CX a, c;CX b, c;}\
CX q[0], q[1];\
CX q[0], q[2];\
CX q[1], q[2];\
__swap__ q[1], q[2];\
CX q[3], q[2];\
__swap__ q[0], q[2];\
CX q[3], q[2];\
CX q[0], q[2];\
";

        Graph* graph = ArchIBMQX2::Create().release();

        std::unique_ptr<QModule> qmod = QModule::ParseString(program, false);
        DependencyBuilderPass* depPass = DependencyBuilderPass::Create(qmod.get());
        DynProgQbitAllocator* allocator = DynProgQbitAllocator::Create
            (qmod.get(), graph, OneRestrictionSwapFinder::Create(graph), depPass);

        allocator->run();

        ReverseEdgesPass* revPass = ReverseEdgesPass::Create(qmod.get(), graph, depPass);
        qmod->runPass(revPass);
        ASSERT_EQ(qmod->toString(), result);
    }
}

TEST(ReverseEdgesPassTests, RevEdgesTest) {
    {
        const std::string program =
"\
gate h a {}\
qreg q[5];\
gate test a, b, c {CX a, b;CX a, c;CX b, c;}\
test q[0], q[1], q[2];\
test q[2], q[1], q[0];\
test q[4], q[1], q[0];\
";
        const std::string result =
"\
gate __swap__ a, b {cx a, b;h a;h b;cx a, b;h a;h b;cx a, b;}\
qreg q[5];\
gate h a {}\
gate test a, b, c {CX a, b;CX a, c;CX b, c;}\
CX q[0], q[1];\
CX q[0], q[2];\
CX q[1], q[2];\
h [2];\
h [1];\
CX q[2], q[1];\
h [1];\
h [2];\
h [2];\
h [0];\
CX q[2], q[0];\
h [0];\
h [2];\
h [1];\
h [0];\
CX q[1], q[0];\
h [0];\
h [1];\
__swap__ q[1], q[2];\
CX q[3], q[2];\
__swap__ q[0], q[2];\
CX q[3], q[2];\
CX q[0], q[2];\
";

        Graph* graph = ArchIBMQX2::Create().release();

        std::unique_ptr<QModule> qmod = QModule::ParseString(program, false);
        DependencyBuilderPass* depPass = DependencyBuilderPass::Create(qmod.get());
        DynProgQbitAllocator* allocator = DynProgQbitAllocator::Create
            (qmod.get(), graph, OneRestrictionSwapFinder::Create(graph), depPass);

        allocator->run();

        /*
        ReverseEdgesPass* revPass = ReverseEdgesPass::Create(qmod.get(), graph, depPass);
        qmod->runPass(revPass);
        ASSERT_EQ(qmod->toString(), result);
        */
    }
}
