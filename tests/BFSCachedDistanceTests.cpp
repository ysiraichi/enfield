
#include "gtest/gtest.h"

#include "enfield/Support/BFSCachedDistance.h"
#include "enfield/Support/uRefCast.h"

#include <string>

using namespace efd;

TEST(BFSCachedDistanceTests, OneDistantTests) {
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

    auto graph = JsonParser<Graph>::ParseString(gStr);
    ASSERT_FALSE(graph.get() == nullptr);

    auto bfsDistance = BFSCachedDistance::Create();
    bfsDistance->init(graph.get());
    ASSERT_EQ(bfsDistance->get(0, 1), (uint32_t) 1);
    ASSERT_EQ(bfsDistance->get(0, 2), (uint32_t) 1);
    ASSERT_EQ(bfsDistance->get(1, 3), (uint32_t) 1);
    ASSERT_EQ(bfsDistance->get(1, 4), (uint32_t) 1);
}

TEST(BFSCachedDistanceTests, UndirectedDistanceTest) {
    const std::string gStr =
"{\
    \"vertices\": 5,\
    \"type\": \"Undirected\",\
    \"adj\": [\
        [ {\"v\": 1}, {\"v\": 2} ],\
        [ {\"v\": 3}, {\"v\": 4} ],\
        [],\
        [],\
        []\
    ]\
}";

    auto graph = JsonParser<Graph>::ParseString(gStr);
    ASSERT_FALSE(graph.get() == nullptr);

    auto bfsDistance = BFSCachedDistance::Create();
    bfsDistance->init(graph.get());
    ASSERT_EQ(bfsDistance->get(1, 0), (uint32_t) 1);
    ASSERT_EQ(bfsDistance->get(2, 0), (uint32_t) 1);
    ASSERT_EQ(bfsDistance->get(3, 0), (uint32_t) 2);
    ASSERT_EQ(bfsDistance->get(4, 0), (uint32_t) 2);
}

TEST(BFSCachedDistanceTests, DirectedDistanceTests) {
    {
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

        auto graph = JsonParser<Graph>::ParseString(gStr);
        ASSERT_FALSE(graph.get() == nullptr);

        auto bfsDistance = BFSCachedDistance::Create();
        bfsDistance->init(graph.get());
        ASSERT_EQ(bfsDistance->get(0, 3), (uint32_t) 2);
        ASSERT_EQ(bfsDistance->get(0, 4), (uint32_t) 2);
    }

    {
        const std::string gStr =
"{\
    \"vertices\": 5,\
    \"type\": \"Undirected\",\
    \"adj\": [\
        [ {\"v\": 1} ],\
        [ {\"v\": 2}],\
        [ {\"v\": 3}],\
        [ {\"v\": 4}],\
        []\
    ]\
}";

        auto graph = JsonParser<Graph>::ParseString(gStr);
        ASSERT_FALSE(graph.get() == nullptr);

        auto bfsDistance = BFSCachedDistance::Create();
        bfsDistance->init(graph.get());
        ASSERT_EQ(bfsDistance->get(4, 0), (uint32_t) 4);
    }
}
