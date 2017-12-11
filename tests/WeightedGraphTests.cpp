
#include "gtest/gtest.h"

#include "enfield/Support/WeightedGraph.h"

#include <string>

using namespace efd;

TEST(GraphTests, SameIntWeightTest) {
    const std::string gStr =
"\
5\n\
0 1 1\n\
0 2 1\n\
1 3 1\n\
1 4 1\n\
";
    auto graph = efd::WeightedGraph<int>::ReadString(gStr);
    ASSERT_FALSE(graph.get() == nullptr);

    ASSERT_EQ(graph->size(), (uint32_t) 5);
    ASSERT_TRUE(graph->hasEdge(0, 1));
    ASSERT_TRUE(graph->hasEdge(0, 2));
    ASSERT_TRUE(graph->hasEdge(1, 3));
    ASSERT_TRUE(graph->hasEdge(1, 4));

    ASSERT_TRUE(graph->getW(0, 1) == 1);
    ASSERT_TRUE(graph->getW(0, 2) == 1);
    ASSERT_TRUE(graph->getW(1, 3) == 1);
    ASSERT_TRUE(graph->getW(1, 4) == 1);
}

TEST(GraphTests, DifferentWeightIntTest) {
    const std::string gStr =
"\
5\n\
0 1 1\n\
1 0 0\n\
0 2 2\n\
1 2 2\n\
1 3 3\n\
1 4 4\n\
4 1 1\n\
";
    auto graph = efd::WeightedGraph<int>::ReadString(gStr);
    ASSERT_FALSE(graph.get() == nullptr);

    ASSERT_EQ(graph->size(), (uint32_t) 5);
    ASSERT_TRUE(graph->hasEdge(0, 1));
    ASSERT_TRUE(graph->hasEdge(1, 0));
    ASSERT_TRUE(graph->hasEdge(0, 2));
    ASSERT_TRUE(graph->hasEdge(1, 2));
    ASSERT_TRUE(graph->hasEdge(1, 3));
    ASSERT_TRUE(graph->hasEdge(1, 4));
    ASSERT_TRUE(graph->hasEdge(4, 1));

    ASSERT_TRUE(graph->getW(0, 1) == 1);
    ASSERT_TRUE(graph->getW(1, 0) == 0);
    ASSERT_TRUE(graph->getW(0, 2) == 2);
    ASSERT_TRUE(graph->getW(1, 2) == 2);
    ASSERT_TRUE(graph->getW(1, 3) == 3);
    ASSERT_TRUE(graph->getW(1, 4) == 4);
    ASSERT_TRUE(graph->getW(4, 1) == 1);
}

TEST(GraphTests, DoubleWeightTest) {
    double episilon = 0.00001;

    const std::string gStr =
"\
5\n\
0 1 1.3\n\
1 0 0.2\n\
0 2 2.8\n\
1 2 2.3\n\
1 3 3.009\n\
1 4 4.1\n\
4 1 3.14159\n\
";
    auto graph = efd::WeightedGraph<double>::ReadString(gStr);
    ASSERT_FALSE(graph.get() == nullptr);

    ASSERT_EQ(graph->size(), (uint32_t) 5);
    ASSERT_TRUE(graph->hasEdge(0, 1));
    ASSERT_TRUE(graph->hasEdge(1, 0));
    ASSERT_TRUE(graph->hasEdge(0, 2));
    ASSERT_TRUE(graph->hasEdge(1, 2));
    ASSERT_TRUE(graph->hasEdge(1, 3));
    ASSERT_TRUE(graph->hasEdge(1, 4));
    ASSERT_TRUE(graph->hasEdge(4, 1));

    ASSERT_TRUE(graph->getW(0, 1) - 1.3 > -episilon);
    ASSERT_TRUE(graph->getW(1, 0) - 0.2 > -episilon);
    ASSERT_TRUE(graph->getW(0, 2) - 2.8 > -episilon);
    ASSERT_TRUE(graph->getW(1, 2) - 2.3 > -episilon);
    ASSERT_TRUE(graph->getW(1, 3) - 3.009 > -episilon);
    ASSERT_TRUE(graph->getW(1, 4) - 4.1 > -episilon);
    ASSERT_TRUE(graph->getW(4, 1) - 3.14159 > -episilon);

    ASSERT_TRUE(graph->getW(0, 1) - 1.3 < episilon);
    ASSERT_TRUE(graph->getW(1, 0) - 0.2 < episilon);
    ASSERT_TRUE(graph->getW(0, 2) - 2.8 < episilon);
    ASSERT_TRUE(graph->getW(1, 2) - 2.3 < episilon);
    ASSERT_TRUE(graph->getW(1, 3) - 3.009 < episilon);
    ASSERT_TRUE(graph->getW(1, 4) - 4.1 < episilon);
    ASSERT_TRUE(graph->getW(4, 1) - 3.14159 < episilon);
}
