
#include "gtest/gtest.h"

#include "enfield/Support/OneRestrictionSwapFinder.h"

#include <string>

using namespace efd;

bool SwapEqual(const SwapFinder::Swap& lhs, const SwapFinder::Swap& rhs) {
    return (lhs.mU == rhs.mU && lhs.mV == rhs.mV) || 
        (lhs.mU == rhs.mV && lhs.mV == rhs.mU);
}

TEST(OneRestrictionSwapFinderTests, NoSwapPathTest) {
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

    SwapFinder::RestrictionVector rV { SwapFinder::Rest { 0, 1 } };
    SwapFinder* finder = OneRestrictionSwapFinder::Create(graph.get());
    SwapFinder::SwapVector swaps = finder->findSwaps(rV);
    ASSERT_TRUE(swaps.empty());
}

TEST(OneRestrictionSwapFinderTests, ReverseEdgeNoSwapPathTest) {
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

    SwapFinder::RestrictionVector rV { SwapFinder::Rest { 3, 1 } };
    SwapFinder* finder = OneRestrictionSwapFinder::Create(graph.get());
    SwapFinder::SwapVector swaps = finder->findSwaps(rV);
    ASSERT_TRUE(swaps.empty());
}

TEST(OneRestrictionSwapFinderTests, SwapTests) {
    {
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

        SwapFinder::RestrictionVector rV { SwapFinder::Rest { 0, 4 } };
        SwapFinder* finder = OneRestrictionSwapFinder::Create(graph.get());
        SwapFinder::SwapVector swaps = finder->findSwaps(rV);
        ASSERT_FALSE(swaps.empty());
        ASSERT_EQ(swaps.size(), 2);
        ASSERT_TRUE(SwapEqual(swaps[0], SwapFinder::Swap { 1, 4 }));
        ASSERT_TRUE(SwapEqual(swaps[1], SwapFinder::Swap { 0, 1 }));
    }

    {
        const std::string gStr =
"\
5\n\
0 1\n\
1 2\n\
2 3\n\
3 4\n\
";
        std::unique_ptr<Graph> graph = efd::Graph::ReadString(gStr);
        ASSERT_FALSE(graph.get() == nullptr);

        SwapFinder::RestrictionVector rV { SwapFinder::Rest { 4, 0 } };
        SwapFinder* finder = OneRestrictionSwapFinder::Create(graph.get());
        SwapFinder::SwapVector swaps = finder->findSwaps(rV);
        ASSERT_FALSE(swaps.empty());
        ASSERT_EQ(swaps.size(), 4);
        ASSERT_TRUE(SwapEqual(swaps[0], SwapFinder::Swap { 0, 1 }));
        ASSERT_TRUE(SwapEqual(swaps[1], SwapFinder::Swap { 1, 2 }));
        ASSERT_TRUE(SwapEqual(swaps[2], SwapFinder::Swap { 2, 3 }));
        ASSERT_TRUE(SwapEqual(swaps[3], SwapFinder::Swap { 3, 4 }));
    }
}
