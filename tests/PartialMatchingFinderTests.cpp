
#include "gtest/gtest.h"

#include "enfield/Support/WeightedPMFinder.h"

#include <string>

using namespace efd;

static void check(const std::string gS, const std::string wGS) {
    Graph::sRef graph = Graph::ReadString(gS);
    auto wGraph = WeightedGraph<int>::ReadString(wGS);
    auto matcher = WeightedPMFinder<int>::Create();
    
    uint32_t gSize = graph->size();
    std::vector<bool> matched(gSize, false);

    // For all 'a' in 'graph'; and 'u' in 'wGraph'
    auto match = matcher->find(graph.get(), wGraph.get());
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

TEST(PartialMatchingFinderTests, NoSegFaultTests) {
    const std::string graphStr = 
"\
5\n\
0 1\n\
0 2\n\
1 2\n\
3 4\n\
3 2\n\
4 2\n\
";

    const std::string wGraphStr = 
"\
5\n\
0 1 1\n\
0 2 1\n\
1 2 1\n\
3 4 1\n\
3 2 1\n\
4 2 1\n\
";

    check(graphStr, wGraphStr);
}

TEST(PartialMatchingFinderTests, VaryingWeightsTest) {
    const std::string graphStr = 
"\
5\n\
0 1\n\
0 2\n\
1 2\n\
3 4\n\
3 2\n\
4 2\n\
";

    const std::string wGraphStr = 
"\
5\n\
0 1 1\n\
0 2 1\n\
1 2 5\n\
3 4 1\n\
3 2 3\n\
4 2 1\n\
";

    check(graphStr, wGraphStr);
}
