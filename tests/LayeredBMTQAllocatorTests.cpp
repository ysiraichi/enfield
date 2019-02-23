
#include "gtest/gtest.h"

#include "enfield/Transform/Allocators/LayeredBMTQAllocator.h"
#include "enfield/Transform/ReverseEdgesPass.h"
#include "enfield/Transform/SemanticVerifierPass.h"
#include "enfield/Transform/ArchVerifierPass.h"
#include "enfield/Transform/PassCache.h"
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

void TestAllocation(const std::string program) {
    static ArchGraph::sRef g(nullptr);
    if (g.get() == nullptr) g = createGraph();

    auto qmod = QModule::ParseString(program);
    auto qmodCopy = qmod->clone();

    auto reverse = ReverseEdgesPass::Create(g);
    auto allocator = LayeredBMTQAllocator::Create(g);
    allocator->run(qmod.get());
    reverse->run(qmod.get());

    auto mapping = allocator->getData();

    qmod->print(std::cout, true);
    qmodCopy->print(std::cout, true);

    auto aVerifierPass = ArchVerifierPass::Create(g);
    PassCache::Run(qmod.get(), aVerifierPass.get());
    EXPECT_TRUE(aVerifierPass->getData());

    auto sVerifierPass = SemanticVerifierPass::Create(std::move(qmodCopy), mapping);
    sVerifierPass->setInlineAll({ "cx" });
    PassCache::Run(qmod.get(), sVerifierPass.get());
    EXPECT_TRUE(sVerifierPass->getData().isSuccess());
}

TEST(SabreQAllocatorTests, SimpleNoSwapProgram) {
    {
        const std::string program =
"\
qreg q[5];\
CX q[0], q[1];\
";
        TestAllocation(program);
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
        TestAllocation(program);
    }
}

TEST(SabreQAllocatorTests, GatesTest) {
    {
        const std::string program =
"\
qreg q[5];\
gate test a, b, c {CX a, b;CX a, c;CX b, c;}\
test q[0], q[1], q[2];\
";
        TestAllocation(program);
    }

    {
        const std::string program =
"\
qreg q[5];\
gate test a, b, c {CX a, b;CX a, c;CX b, c;}\
test q[0], q[1], q[2];\
test q[3], q[4], q[2];\
";
        TestAllocation(program);
    }
}

TEST(SabreQAllocatorTests, GatesSwapTest) {
    {
        const std::string program =
"\
qreg q[5];\
gate test a, b, c {CX a, b;CX a, c;CX b, c;}\
test q[0], q[1], q[2];\
test q[4], q[1], q[0];\
";
        TestAllocation(program);
    }
}
