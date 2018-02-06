#include "gtest/gtest.h"

#include "enfield/Support/Graph.h"
#include "enfield/Support/WeightedGraph.h"

#include <string>

using namespace efd;

template <typename T = efd::Graph>
void TestDotifyTrue(std::string gstr, std::string result,
                    Graph::Type ty = Graph::Undirected) {
    auto graph = T::ReadString(gstr, ty);
    ASSERT_FALSE(graph.get() == nullptr);
    ASSERT_EQ(graph->dotify(), result);
}

template <typename T = efd::Graph>
void TestDotifyFalse(std::string gstr, std::string result,
                     Graph::Type ty = Graph::Undirected) {
    auto graph = T::ReadString(gstr, ty);
    ASSERT_FALSE(graph.get() == nullptr);
    ASSERT_NE(graph->dotify(), result);
}

TEST(GraphDotifyTests, UndirectedGraphTest) {
    {
        const std::string gStr =
"5\n\
0 1\n\
0 2\n\
1 3\n\
1 4\n";

        const std::string result =
"graph Graph {\n\
    0;\n\
    0 -- 1;\n\
    0 -- 2;\n\
    1;\n\
    1 -- 3;\n\
    1 -- 4;\n\
    2;\n\
    3;\n\
    4;\n\
}";

        TestDotifyTrue(gStr, result);
    }
    {
        const std::string gStr =
"5\n\
0 1\n\
1 0\n\
0 2\n\
1 2\n\
1 3\n\
1 4\n\
4 1\n";

        const std::string result =
"graph Graph {\n\
    0;\n\
    0 -- 1;\n\
    0 -- 2;\n\
    1;\n\
    1 -- 2;\n\
    1 -- 3;\n\
    1 -- 4;\n\
    2;\n\
    3;\n\
    4;\n\
}";

        TestDotifyTrue(gStr, result);
    }
}

TEST(GraphDotifyTests, DirectedGraphTest) {
    {
        const std::string gStr =
"5\n\
0 1\n\
2 0\n\
1 3\n\
4 1\n";

        const std::string result =
"digraph Graph {\n\
    0;\n\
    0 -> 1;\n\
    1;\n\
    1 -> 3;\n\
    2;\n\
    2 -> 0;\n\
    3;\n\
    4;\n\
    4 -> 1;\n\
}";

        TestDotifyTrue(gStr, result, Graph::Directed);
    }
    {
        const std::string gStr =
"5\n\
0 1\n\
1 0\n\
0 2\n\
1 2\n\
3 1\n\
4 1\n\
1 4\n";

        const std::string result =
"digraph Graph {\n\
    0;\n\
    0 -> 1;\n\
    0 -> 2;\n\
    1;\n\
    1 -> 0;\n\
    1 -> 2;\n\
    1 -> 4;\n\
    2;\n\
    3;\n\
    3 -> 1;\n\
    4;\n\
    4 -> 1;\n\
}";

        TestDotifyTrue(gStr, result, Graph::Directed);
    }
}

TEST(GraphDotifyTests, UndirectedWeightedGraphTest) {
    {
        const std::string gStr =
"5\n\
0 1 1\n\
0 2 1\n\
1 3 1\n\
1 4 1\n";

        const std::string result =
"graph Graph {\n\
    0;\n\
    0 -- 1[label=1];\n\
    0 -- 2[label=1];\n\
    1;\n\
    1 -- 3[label=1];\n\
    1 -- 4[label=1];\n\
    2;\n\
    3;\n\
    4;\n\
}";

        TestDotifyTrue<WeightedGraph<uint32_t>>(gStr, result);
    }
    {
        const std::string gStr =
"5\n\
0 1 1\n\
0 2 2\n\
1 2 2\n\
1 3 3\n\
4 1 1\n";

        const std::string result =
"graph Graph {\n\
    0;\n\
    0 -- 1[label=1];\n\
    0 -- 2[label=2];\n\
    1;\n\
    1 -- 2[label=2];\n\
    1 -- 3[label=3];\n\
    1 -- 4[label=1];\n\
    2;\n\
    3;\n\
    4;\n\
}";

        TestDotifyTrue<WeightedGraph<uint32_t>>(gStr, result);
    }
}

TEST(GraphDotifyTests, DirectedWeightedGraphTest) {
    {
        const std::string gStr =
"5\n\
0 1 1\n\
4 1 1\n\
2 0 1\n\
1 3 1\n";

        const std::string result =
"digraph Graph {\n\
    0;\n\
    0 -> 1[label=1];\n\
    1;\n\
    1 -> 3[label=1];\n\
    2;\n\
    2 -> 0[label=1];\n\
    3;\n\
    4;\n\
    4 -> 1[label=1];\n\
}";

        TestDotifyTrue<WeightedGraph<uint32_t>>(gStr, result, Graph::Directed);
    }
    {
        const std::string gStr =
"5\n\
0 1 1\n\
1 0 2\n\
1 2 2\n\
2 1 3\n\
4 2 1\n\
2 4 3\n";

        const std::string result =
"digraph Graph {\n\
    0;\n\
    0 -> 1[label=1];\n\
    1;\n\
    1 -> 0[label=2];\n\
    1 -> 2[label=2];\n\
    2;\n\
    2 -> 1[label=3];\n\
    2 -> 4[label=3];\n\
    3;\n\
    4;\n\
    4 -> 2[label=1];\n\
}";

        TestDotifyTrue<WeightedGraph<uint32_t>>(gStr, result, Graph::Directed);
    }
}
