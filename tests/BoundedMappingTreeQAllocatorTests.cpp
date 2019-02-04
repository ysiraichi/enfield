
#include "gtest/gtest.h"

#include "enfield/Transform/Allocators/BoundedMappingTreeQAllocator.h"
#include "enfield/Transform/Allocators/BMT/DefaultBMTQAllocatorImpl.h"
#include "enfield/Transform/Allocators/BMT/ImprovedBMTQAllocatorImpl.h"
#include "enfield/Transform/SemanticVerifierPass.h"
#include "enfield/Transform/ArchVerifierPass.h"
#include "enfield/Transform/PassCache.h"
#include "enfield/Arch/ArchGraph.h"
#include "enfield/Support/RTTI.h"
#include "enfield/Support/uRefCast.h"
#include "enfield/Support/ApproxTSFinder.h"

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

void FillBMT(BoundedMappingTreeQAllocator::Ref allocator) {
    allocator->setNodeCandidatesGenerator(SeqNCandidatesGenerator::Create());\
    allocator->setChildrenSelector(FirstCandidateSelector::Create());\
    allocator->setPartialSolutionSelector(FirstCandidateSelector::Create());\
    allocator->setSwapCostEstimator(GeoDistanceSwapCEstimator::Create());\
    allocator->setLiveQubitsPreProcessor(GeoNearestLQPProcessor::Create());\
    allocator->setMapSeqSelector(BestNMSSelector::Create());\
    allocator->setTokenSwapFinder(ApproxTSFinder::Create());\
}

void FillIBMT(BoundedMappingTreeQAllocator::Ref allocator) {
    allocator->setNodeCandidatesGenerator(CircuitCandidatesGenerator::Create());\
    allocator->setChildrenSelector(WeightedRouletteCandidateSelector::Create());\
    allocator->setPartialSolutionSelector(WeightedRouletteCandidateSelector::Create());\
    allocator->setSwapCostEstimator(GeoDistanceSwapCEstimator::Create());\
    allocator->setLiveQubitsPreProcessor(GeoNearestLQPProcessor::Create());\
    allocator->setMapSeqSelector(BestNMSSelector::Create());\
    allocator->setTokenSwapFinder(ApproxTSFinder::Create());\
}

void TestAllocator(QModule::Ref qmod, ArchGraph::sRef g, QbitAllocator::Ref allocator) {
    auto qmodCopy = qmod->clone();
    allocator->run(qmod);

    auto mapping = allocator->getData();

    qmod->print(std::cout, true);
    qmodCopy->print(std::cout, true);

    auto aVerifierPass = ArchVerifierPass::Create(g);
    PassCache::Run(qmod, aVerifierPass.get());
    EXPECT_TRUE(aVerifierPass->getData());

    auto sVerifierPass = SemanticVerifierPass::Create(std::move(qmodCopy), mapping);
    sVerifierPass->setInlineAll({ "cx" });
    PassCache::Run(qmod, sVerifierPass.get());
    EXPECT_TRUE(sVerifierPass->getData().isSuccess());
}

void TestAllocation(const std::string program) {
    static ArchGraph::sRef g(nullptr);
    if (g.get() == nullptr) g = createGraph();

    auto qmod = QModule::ParseString(program);

    {
        auto allocator = BoundedMappingTreeQAllocator::Create(g);
        FillBMT(allocator.get());
        TestAllocator(qmod.get(), g, allocator.get());
    }
    {
        auto allocator = BoundedMappingTreeQAllocator::Create(g);
        FillIBMT(allocator.get());
        TestAllocator(qmod.get(), g, allocator.get());
    }
}

TEST(BoundedMappingTreeQAllocatorTests, SimpleNoSwapProgram) {
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
        // Expected [ 2 1 0 4 3 ]
        TestAllocation(program);
    }
}

TEST(BoundedMappingTreeQAllocatorTests, GatesTest) {
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

TEST(BoundedMappingTreeQAllocatorTests, GatesSwapTest) {
    {
        const std::string program =
"\
qreg q[5];\
gate test a, b, c {CX a, b;CX a, c;CX b, c;}\
test q[0], q[1], q[2];\
test q[4], q[1], q[0];\
";
        // Expected mapping: [ 0 2 1 3 4 ]
        TestAllocation(program);
    }
}
