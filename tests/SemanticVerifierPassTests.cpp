
#include "gtest/gtest.h"

#include "enfield/Transform/SemanticVerifierPass.h"
#include "enfield/Transform/Allocators/SimpleDepSolver.h"
#include "enfield/Transform/Allocators/WeightedSIMappingFinder.h"
#include "enfield/Transform/Allocators/PathGuidedSolBuilder.h"
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

TEST(WeightedSIMappingFinderTests, SimpleNoSwapProgram) {
    {
        const std::string progBefore =
"\
qreg q[5];\
CX q[0], q[1];\
";
        const std::string progAfter =
"\
include \"qelib1.inc\";\
qreg q[5];\
CX q[1], q[2];\
";

        ArchGraph::sRef graph = getGraph();

        auto qmodBefore = QModule::ParseString(progBefore);
        auto qmodAfter = QModule::ParseString(progAfter);

        SemanticVerifierPass verifier(std::move(qmodBefore), Mapping { 1, 2, 0, 3, 4 });
        verifier.run(qmodAfter.get());

        ASSERT_TRUE(verifier.getData());
    }

    {
        const std::string progBefore =
"\
qreg q[5];\
CX q[2], q[1];\
CX q[2], q[0];\
CX q[1], q[0];\
CX q[4], q[3];\
CX q[4], q[0];\
CX q[3], q[0];\
";
        const std::string progAfter =
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

        ArchGraph::sRef graph = getGraph();

        auto qmodBefore = QModule::ParseString(progBefore);
        auto qmodAfter = QModule::ParseString(progAfter);

        SemanticVerifierPass verifier(std::move(qmodBefore), Mapping { 2, 4, 3, 1, 0 });
        verifier.run(qmodAfter.get());

        ASSERT_TRUE(verifier.getData());
    }
}

TEST(WeightedSIMappingFinderTests, GatesTest) {
    {
        const std::string progBefore =
"\
qreg q[5];\
gate test a, b, c {CX a, b;CX a, c;CX b, c;}\
test q[0], q[1], q[2];\
";
        const std::string progAfter =
"\
include \"qelib1.inc\";\
qreg q[5];\
CX q[0], q[1];\
CX q[0], q[2];\
CX q[1], q[2];\
";

        ArchGraph::sRef graph = getGraph();

        auto qmodBefore = QModule::ParseString(progBefore);
        auto qmodAfter = QModule::ParseString(progAfter);

        SemanticVerifierPass verifier(std::move(qmodBefore), Mapping { 0, 1, 2, 3, 4 });
        verifier.run(qmodAfter.get());

        ASSERT_TRUE(verifier.getData());
    }

    {
        const std::string progBefore =
"\
qreg q[5];\
gate test a, b, c {CX a, b;CX a, c;CX b, c;}\
test q[0], q[1], q[2];\
test q[3], q[4], q[2];\
";
        const std::string progAfter =
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

        ArchGraph::sRef graph = getGraph();

        auto qmodBefore = QModule::ParseString(progBefore);
        auto qmodAfter = QModule::ParseString(progAfter);

        SemanticVerifierPass verifier(std::move(qmodBefore), Mapping { 3, 4, 2, 0, 1 });
        verifier.run(qmodAfter.get());

        ASSERT_TRUE(verifier.getData());
    }
}

TEST(WeightedSIMappingFinderTests, GatesSwapTest) {
    {
        const std::string progBefore =
"\
qreg q[5];\
gate test a, b, c {CX a, b;CX a, c;CX b, c;}\
test q[0], q[1], q[2];\
test q[4], q[1], q[0];\
";
        // Expected mapping: [ 0 2 1 3 4 ]
        const std::string progAfter =
"\
include \"qelib1.inc\";\
gate intrinsic_swap__ a, b {cx a, b;cx b, a;cx a, b;}\
gate intrinsic_rev_cx__ a, b {h a;h b;cx b, a;h b;h a;}\
qreg q[5];\
CX q[1], q[2];\
intrinsic_swap__ q[3], q[2];\
CX q[1], q[2];\
CX q[3], q[2];\
intrinsic_swap__ q[3], q[2];\
CX q[0], q[2];\
CX q[0], q[1];\
intrinsic_rev_cx__ q[2], q[1];\
";

        ArchGraph::sRef graph = getGraph();

        auto qmodBefore = QModule::ParseString(progBefore);
        auto qmodAfter = QModule::ParseString(progAfter);

        SemanticVerifierPass verifier(std::move(qmodBefore), Mapping { 1, 2, 3, 4, 0 });
        verifier.run(qmodAfter.get());

        ASSERT_TRUE(verifier.getData());
    }
}
