
#include "gtest/gtest.h"

#include "enfield/Support/BFSPathFinder.h"
#include "enfield/Support/uRefCast.h"

#include <string>

using namespace efd;

void PathEqual(const std::vector<uint32_t>& lhs, const std::vector<uint32_t>& rhs) {
    ASSERT_EQ(lhs.size(), rhs.size());
    for (uint32_t i = 0, e = lhs.size(); i < e; ++i)
        ASSERT_EQ(lhs[i], rhs[i]);
}

TEST(BFSPathFinderTests, NoSwapPathTest) {
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

    auto finder = BFSPathFinder::Create();
    auto path = finder->find(graph.get(), 0, 1);;
    ASSERT_EQ(path.size(), (uint32_t) 2);
}

TEST(BFSPathFinderTests, ReverseEdgeNoSwapPathTest) {
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

    auto finder = BFSPathFinder::Create();
    auto path = finder->find(graph.get(), 3, 1);
    ASSERT_EQ(path.size(), (uint32_t) 2);
}

TEST(BFSPathFinderTests, SwapTests) {
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

        auto finder = BFSPathFinder::Create();
        auto path = finder->find(graph.get(), 0, 4);;
        ASSERT_EQ(path.size(), (uint32_t) 3);
        PathEqual(path, { 0, 1, 4 });
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

        auto finder = BFSPathFinder::Create();
        auto path = finder->find(graph.get(), 4, 0);
        ASSERT_EQ(path.size(), (uint32_t) 5);
        PathEqual(path, { 4, 3, 2, 1, 0 });
    }
}
