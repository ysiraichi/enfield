
#include "gtest/gtest.h"

#include "enfield/Support/WeightedSIFinder.h"
#include "enfield/Support/JsonParser.h"

#include <string>

using namespace efd;

static void check(const std::string gS, const std::string wGS) {
    Graph::sRef graph = JsonParser<Graph>::ParseString(gS);
    auto wGraph = JsonParser<WeightedGraph<int>>::ParseString(wGS);
    auto matcher = WeightedSIFinder<int>::Create();
    
    uint32_t gSize = graph->size();
    std::vector<bool> matched(gSize, false);

    // For all 'a' in 'graph'; and 'u' in 'wGraph'
    auto match = matcher->find(graph.get(), wGraph.get()).m;
    for (uint32_t a = 0; a < match.size(); ++a) {
        // 'a' must be assigned to a 'u' >= 0
        ASSERT_TRUE(match[a] >= 0);
        // 'a' can't be assigned to a 'u' >= gSize.
        ASSERT_TRUE(match[a] < gSize);
        // if 'a' is assigned to 'u', there can't be a 'b' != 'a',
        // such that 'b' is also assigned to 'u'.
        ASSERT_FALSE(matched[match[a]]);

        matched[match[a]] = true;

        std::cerr << std::to_string(a) << " -> " << std::to_string(match[a]) << std::endl;
    }
}

TEST(SIFinderTests, NoSegFaultTests) {
    const std::string graphStr = 
"{\
    \"vertices\": 5,\
    \"type\": \"Directed\",\
    \"adj\": [\
        [ {\"v\": 1}, {\"v\": 2} ],\
        [ {\"v\": 2} ],\
        [],\
        [ {\"v\": 2}, {\"v\": 4} ],\
        [ {\"v\": 2} ]\
    ]\
}";

    const std::string wGraphStr = 
"{\
    \"vertices\": 5,\
    \"type\": \"Directed\",\
    \"adj\": [\
        [ {\"v\": 1, \"w\": 1}, {\"v\": 2, \"w\": 1} ],\
        [ {\"v\": 2, \"w\": 1} ],\
        [],\
        [ {\"v\": 2, \"w\": 1}, {\"v\": 4, \"w\": 1} ],\
        [ {\"v\": 2, \"w\": 1} ]\
    ]\
}";

    check(graphStr, wGraphStr);
}

TEST(SIFinderTests, VaryingWeightsTest) {
    const std::string graphStr = 
"{\
    \"vertices\": 5,\
    \"type\": \"Directed\",\
    \"adj\": [\
        [ {\"v\": 1}, {\"v\": 2} ],\
        [ {\"v\": 2} ],\
        [],\
        [ {\"v\": 2}, {\"v\": 4} ],\
        [ {\"v\": 2} ]\
    ]\
}";

    const std::string wGraphStr = 
"{\
    \"vertices\": 5,\
    \"type\": \"Directed\",\
    \"adj\": [\
        [ {\"v\": 1, \"w\": 1}, {\"v\": 2, \"w\": 1} ],\
        [ {\"v\": 2, \"w\": 5} ],\
        [],\
        [ {\"v\": 2, \"w\": 3}, {\"v\": 4, \"w\": 1} ],\
        [ {\"v\": 2, \"w\": 1} ]\
    ]\
}";

    check(graphStr, wGraphStr);
}
