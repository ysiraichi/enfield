
#include "gtest/gtest.h"

#include "enfield/Transform/DependencyGraphBuilderPass.h"
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

void TestBuiltGraph(const std::string program, const std::string result) {
    static ArchGraph::sRef g(nullptr);
    if (g.get() == nullptr) g = createGraph();

    auto qmod = QModule::ParseString(program);

    auto builderPass = PassCache::Get<DependencyGraphBuilderPass>(qmod.get());
    auto depgraph = builderPass->getData();

    EXPECT_EQ(depgraph->dotify(), result);
}

TEST(DependencyGraphBuilderPassTests, SingleDependencyProgram) {
    {
        const std::string program =
"\
qreg q[2];\
CX q[0], q[1];\
";
        const std::string result =
"digraph Dump {\n\
    0;\n\
    0 -> 1[label=1];\n\
    1;\n\
}";
        TestBuiltGraph(program, result);
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
        const std::string result =
"digraph Dump {\n\
    0;\n\
    1;\n\
    1 -> 0[label=1];\n\
    2;\n\
    2 -> 0[label=1];\n\
    2 -> 1[label=1];\n\
    3;\n\
    3 -> 0[label=1];\n\
    4;\n\
    4 -> 0[label=1];\n\
    4 -> 3[label=1];\n\
}";

        TestBuiltGraph(program, result);
    }
}

TEST(DependencyGraphBuilderPassTests, MultipleDependencyProgram) {
    {
        const std::string program =
"\
qreg q[2];\
CX q[0], q[1];\
CX q[0], q[1];\
CX q[0], q[1];\
CX q[0], q[1];\
";
        const std::string result =
"digraph Dump {\n\
    0;\n\
    0 -> 1[label=4];\n\
    1;\n\
}";
        TestBuiltGraph(program, result);
    }

    {
        const std::string program =
"\
qreg q[5];\
CX q[2], q[1];\
CX q[2], q[0];\
CX q[1], q[0];\
CX q[4], q[3];\
CX q[2], q[1];\
CX q[3], q[0];\
CX q[4], q[3];\
CX q[3], q[0];\
CX q[3], q[0];\
CX q[4], q[3];\
CX q[2], q[1];\
CX q[4], q[0];\
CX q[3], q[0];\
";
        const std::string result =
"digraph Dump {\n\
    0;\n\
    1;\n\
    1 -> 0[label=1];\n\
    2;\n\
    2 -> 0[label=1];\n\
    2 -> 1[label=3];\n\
    3;\n\
    3 -> 0[label=4];\n\
    4;\n\
    4 -> 0[label=1];\n\
    4 -> 3[label=3];\n\
}";

        TestBuiltGraph(program, result);
    }
}

TEST(DependencyGraphBuilderPassTests, GatesTest) {
    {
        const std::string program =
"\
qreg q[5];\
gate test a, b, c {CX a, b;CX a, c;CX b, c;}\
test q[0], q[1], q[2];\
";
        const std::string result =
"digraph Dump {\n\
    0;\n\
    0 -> 1[label=1];\n\
    0 -> 2[label=1];\n\
    1;\n\
    1 -> 2[label=1];\n\
    2;\n\
    3;\n\
    4;\n\
}";
        TestBuiltGraph(program, result);
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
"digraph Dump {\n\
    0;\n\
    0 -> 1[label=1];\n\
    0 -> 2[label=1];\n\
    1;\n\
    1 -> 2[label=1];\n\
    2;\n\
    3;\n\
    3 -> 2[label=1];\n\
    3 -> 4[label=1];\n\
    4;\n\
    4 -> 2[label=1];\n\
}";
        TestBuiltGraph(program, result);
    }
    {
        const std::string program =
"\
qreg q[5];\
gate test a, b, c {CX a, b;CX a, c;CX b, c;}\
test q[0], q[1], q[2];\
test q[4], q[1], q[0];\
";
        const std::string result =
"digraph Dump {\n\
    0;\n\
    0 -> 1[label=1];\n\
    0 -> 2[label=1];\n\
    1;\n\
    1 -> 0[label=1];\n\
    1 -> 2[label=1];\n\
    2;\n\
    3;\n\
    4;\n\
    4 -> 0[label=1];\n\
    4 -> 1[label=1];\n\
}";
        TestBuiltGraph(program, result);
    }
}
