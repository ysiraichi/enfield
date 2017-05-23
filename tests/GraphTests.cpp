
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
    std::unique_ptr<Graph> graph = efd::Graph::ReadString(gStr);
    ASSERT_FALSE(graph.get() == nullptr);

    ASSERT_EQ(graph->size(), 5);
    ASSERT_TRUE(graph->hasEdge(0, 1));
    ASSERT_TRUE(graph->hasEdge(0, 2));
    ASSERT_TRUE(graph->hasEdge(1, 3));
    ASSERT_TRUE(graph->hasEdge(1, 4));

    ASSERT_TRUE(graph->hasEdge(1, 0));
    ASSERT_TRUE(graph->hasEdge(2, 0));
    ASSERT_TRUE(graph->hasEdge(3, 1));
    ASSERT_TRUE(graph->hasEdge(4, 1));
    ASSERT_TRUE(graph->isReverseEdge(1, 0));
    ASSERT_TRUE(graph->isReverseEdge(2, 0));
    ASSERT_TRUE(graph->isReverseEdge(3, 1));
    ASSERT_TRUE(graph->isReverseEdge(4, 1));
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
    std::unique_ptr<Graph> graph = efd::Graph::ReadString(gStr);
    ASSERT_FALSE(graph.get() == nullptr);

    ASSERT_EQ(graph->size(), 5);
    ASSERT_TRUE(graph->hasEdge(0, 1));
    ASSERT_TRUE(graph->hasEdge(1, 0));
    ASSERT_TRUE(graph->hasEdge(0, 2));
    ASSERT_TRUE(graph->hasEdge(1, 2));
    ASSERT_TRUE(graph->hasEdge(1, 3));
    ASSERT_TRUE(graph->hasEdge(1, 4));
    ASSERT_TRUE(graph->hasEdge(4, 1));

    ASSERT_TRUE(graph->hasEdge(2, 0));
    ASSERT_TRUE(graph->hasEdge(2, 1));
    ASSERT_TRUE(graph->hasEdge(3, 1));
    ASSERT_TRUE(graph->isReverseEdge(2, 0));
    ASSERT_TRUE(graph->isReverseEdge(2, 1));
    ASSERT_TRUE(graph->isReverseEdge(3, 1));

    ASSERT_FALSE(graph->isReverseEdge(1, 0));
    ASSERT_FALSE(graph->isReverseEdge(4, 1));
}

TEST(GraphTests, StringIdTest) {
    const std::string gStr =
"\
5\n\
red blue\n\
blue red\n\
red green\n\
blue green\n\
blue yellow\n\
blue purple\n\
purple blue\n\
";
    std::unique_ptr<Graph> graph = efd::Graph::ReadString(gStr);
    ASSERT_FALSE(graph.get() == nullptr);

    ASSERT_EQ(graph->size(), 5);
    ASSERT_TRUE(graph->hasEdge(graph->getUId("red"), graph->getUId("blue")));
    ASSERT_TRUE(graph->hasEdge(graph->getUId("blue"), graph->getUId("red")));
    ASSERT_TRUE(graph->hasEdge(graph->getUId("red"), graph->getUId("green")));
    ASSERT_TRUE(graph->hasEdge(graph->getUId("blue"), graph->getUId("green")));
    ASSERT_TRUE(graph->hasEdge(graph->getUId("blue"), graph->getUId("yellow")));
    ASSERT_TRUE(graph->hasEdge(graph->getUId("blue"), graph->getUId("purple")));
    ASSERT_TRUE(graph->hasEdge(graph->getUId("purple"), graph->getUId("blue")));

    ASSERT_TRUE(graph->hasEdge(graph->getUId("green"), graph->getUId("red")));
    ASSERT_TRUE(graph->hasEdge(graph->getUId("green"), graph->getUId("blue")));
    ASSERT_TRUE(graph->hasEdge(graph->getUId("yellow"), graph->getUId("blue")));
    ASSERT_TRUE(graph->isReverseEdge(graph->getUId("green"), graph->getUId("red")));
    ASSERT_TRUE(graph->isReverseEdge(graph->getUId("green"), graph->getUId("blue")));
    ASSERT_TRUE(graph->isReverseEdge(graph->getUId("yellow"), graph->getUId("blue")));

    ASSERT_FALSE(graph->isReverseEdge(graph->getUId("blue"), graph->getUId("red")));
    ASSERT_FALSE(graph->isReverseEdge(graph->getUId("purple"), graph->getUId("blue")));
}
