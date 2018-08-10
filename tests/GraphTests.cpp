
#include "gtest/gtest.h"

#include "enfield/Support/Graph.h"
#include "enfield/Support/JsonParser.h"

#include <string>

using namespace efd;

TEST(GraphTests, TreeCreationTest) {
    const std::string gStr =
"{\
    \"vertices\": 5,\
    \"type\": \"Directed\",\
    \"adj\": [\
        [ {\"v\": 1}, {\"v\": 2} ],\
        [ {\"v\": 3}, {\"v\": 4} ],\
        [],\
        [],\
        []\
    ]\
}";
    auto graph = efd::JsonParser<efd::Graph>::ParseString(gStr);
    ASSERT_FALSE(graph.get() == nullptr);

    ASSERT_EQ(graph->size(), (uint32_t) 5);
    ASSERT_TRUE(graph->hasEdge(0, 1));
    ASSERT_TRUE(graph->hasEdge(0, 2));
    ASSERT_TRUE(graph->hasEdge(1, 3));
    ASSERT_TRUE(graph->hasEdge(1, 4));
}

TEST(GraphTests, SomeReverseEdgesTest) {
    const std::string gStr =
"{\
    \"vertices\": 5,\
    \"type\": \"Directed\",\
    \"adj\": [\
        [ {\"v\": 1}, {\"v\": 2} ],\
        [ {\"v\": 0}, {\"v\": 2}, {\"v\": 3}, {\"v\": 4} ],\
        [],\
        [],\
        [ {\"v\": 1} ]\
    ]\
}";
    auto graph = efd::JsonParser<efd::Graph>::ParseString(gStr);
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
