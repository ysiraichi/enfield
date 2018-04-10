#include "gtest/gtest.h"

#include "enfield/Arch/ArchGraph.h"

#include <string>

using namespace efd;

TEST(ArchGraphTests, TreeCreationTest) {
    const std::string gStr =
"\
1 5\n\
q 5\n\
q[0] q[1]\n\
q[0] q[2]\n\
q[1] q[3]\n\
q[1] q[4]\n\
";
    std::unique_ptr<ArchGraph> graph = efd::ArchGraph::ReadString(gStr);
    ASSERT_FALSE(graph.get() == nullptr);

    ASSERT_EQ(graph->size(), (uint32_t) 5);
    ASSERT_TRUE(graph->hasEdge(0, 1));
    ASSERT_TRUE(graph->hasEdge(0, 2));
    ASSERT_TRUE(graph->hasEdge(1, 3));
    ASSERT_TRUE(graph->hasEdge(1, 4));

    ASSERT_TRUE(graph->isReverseEdge(1, 0));
    ASSERT_TRUE(graph->isReverseEdge(2, 0));
    ASSERT_TRUE(graph->isReverseEdge(3, 1));
    ASSERT_TRUE(graph->isReverseEdge(4, 1));
}

TEST(ArchGraphTests, SomeReverseEdgesTest) {
    const std::string gStr =
"\
1 5\n\
q 5\n\
q[0] q[1]\n\
q[1] q[0]\n\
q[0] q[2]\n\
q[1] q[2]\n\
q[1] q[3]\n\
q[1] q[4]\n\
q[4] q[1]\n\
";
    std::unique_ptr<ArchGraph> graph = efd::ArchGraph::ReadString(gStr);
    ASSERT_FALSE(graph.get() == nullptr);

    ASSERT_EQ(graph->size(), (uint32_t) 5);
    ASSERT_TRUE(graph->hasEdge(0, 1));
    ASSERT_TRUE(graph->hasEdge(1, 0));
    ASSERT_TRUE(graph->hasEdge(0, 2));
    ASSERT_TRUE(graph->hasEdge(1, 2));
    ASSERT_TRUE(graph->hasEdge(1, 3));
    ASSERT_TRUE(graph->hasEdge(1, 4));
    ASSERT_TRUE(graph->hasEdge(4, 1));

    ASSERT_TRUE(graph->isReverseEdge(2, 0));
    ASSERT_TRUE(graph->isReverseEdge(2, 1));
    ASSERT_TRUE(graph->isReverseEdge(3, 1));

    ASSERT_FALSE(graph->isReverseEdge(1, 0));
    ASSERT_FALSE(graph->isReverseEdge(4, 1));
}
