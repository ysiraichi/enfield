
#include "gtest/gtest.h"

#include "enfield/Support/BFSPathFinder.h"
#include "enfield/Support/uRefCast.h"

#include <string>

using namespace efd;

void PathEqual(const std::vector<unsigned>& lhs, const std::vector<unsigned>& rhs) {
    ASSERT_EQ(lhs.size(), rhs.size());
    for (unsigned i = 0, e = lhs.size(); i < e; ++i)
        ASSERT_EQ(lhs[i], rhs[i]);
}

TEST(BFSPathFinderTests, NoSwapPathTest) {
    const std::string gStr =
"\
5\n\
0 1\n\
0 2\n\
1 3\n\
1 4\n\
";
    auto graph = efd::toShared(efd::Graph::ReadString(gStr));
    ASSERT_FALSE(graph.get() == nullptr);

    auto finder = BFSPathFinder::Create(graph);
    auto path = finder->find(0, 1);;
    ASSERT_EQ(path.size(), 2);
}

TEST(BFSPathFinderTests, ReverseEdgeNoSwapPathTest) {
    const std::string gStr =
"\
5\n\
0 1\n\
0 2\n\
1 3\n\
1 4\n\
";
    auto graph = efd::toShared(efd::Graph::ReadString(gStr));
    ASSERT_FALSE(graph.get() == nullptr);

    auto finder = BFSPathFinder::Create(graph);
    auto path = finder->find(3, 1);
    ASSERT_EQ(path.size(), 2);
}

TEST(BFSPathFinderTests, SwapTests) {
    {
        const std::string gStr =
"\
5\n\
0 1\n\
0 2\n\
1 3\n\
1 4\n\
";
        auto graph = efd::toShared(efd::Graph::ReadString(gStr));
        ASSERT_FALSE(graph.get() == nullptr);

        auto finder = BFSPathFinder::Create(graph);
        auto path = finder->find(0, 4);;
        ASSERT_EQ(path.size(), 3);
        PathEqual(path, { 0, 1, 4 });
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
        auto graph = efd::toShared(efd::Graph::ReadString(gStr));
        ASSERT_FALSE(graph.get() == nullptr);

        auto finder = BFSPathFinder::Create(graph);
        auto path = finder->find(4, 0);
        ASSERT_EQ(path.size(), 5);
        PathEqual(path, { 4, 3, 2, 1, 0 });
    }
}
