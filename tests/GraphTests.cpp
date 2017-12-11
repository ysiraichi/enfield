
#include "gtest/gtest.h"

#include "enfield/Support/Graph.h"

#include <string>

using namespace efd;

TEST(GraphTests, TreeCreationTest) {
    const std::string gStr =
"\
5\n\
0 1\n\
0 2\n\
1 3\n\
1 4\n\
";
    auto graph = efd::Graph::ReadString(gStr);
    ASSERT_FALSE(graph.get() == nullptr);

    ASSERT_EQ(graph->size(), (uint32_t) 5);
    ASSERT_TRUE(graph->hasEdge(0, 1));
    ASSERT_TRUE(graph->hasEdge(0, 2));
    ASSERT_TRUE(graph->hasEdge(1, 3));
    ASSERT_TRUE(graph->hasEdge(1, 4));
}

TEST(GraphTests, SomeReverseEdgesTest) {
    const std::string gStr =
"\
5\n\
0 1\n\
1 0\n\
0 2\n\
1 2\n\
1 3\n\
1 4\n\
4 1\n\
";
    auto graph = efd::Graph::ReadString(gStr);
    ASSERT_FALSE(graph.get() == nullptr);

    ASSERT_EQ(graph->size(), (uint32_t) 5);
    ASSERT_TRUE(graph->hasEdge(0, 1));
    ASSERT_TRUE(graph->hasEdge(1, 0));
    ASSERT_TRUE(graph->hasEdge(0, 2));
    ASSERT_TRUE(graph->hasEdge(1, 2));
    ASSERT_TRUE(graph->hasEdge(1, 3));
    ASSERT_TRUE(graph->hasEdge(1, 4));
    ASSERT_TRUE(graph->hasEdge(4, 1));
}
