#include "gtest/gtest.h"
#include "enfield/Support/ApproxTSFinder.h"
#include "enfield/Support/ExpTSFinder.h"

using namespace efd;

static Graph::sRef graph;
static const std::string graphstr =
"\
5\n\
0 1\n\
0 2\n\
1 2\n\
3 4\n\
3 2\n\
4 2\
";

static Graph::sRef GetGraph() {
    if (graph.get() == nullptr) {
        graph = Graph::ReadString(graphstr);
    }

    return graph;
}

static void CheckSwapSeq(Assign from, Assign to) {
    auto graph = GetGraph();
    ApproxTSFinder approxfinder(graph);
    ExpTSFinder expfinder(graph);

    auto approxseq = approxfinder.find(from, to);
    auto expseq = expfinder.find(from, to);

    ASSERT_TRUE(approxseq.size() <= 4 * expseq.size());

    if (approxseq.size() != expseq.size()) {
        std::cerr << "4-Approx: " << approxseq.size() << std::endl;
        std::cerr << "Exp: " << expseq.size() << std::endl;
    }

    if (!approxseq.empty()) {
        for (auto swap : approxseq)
            std::swap(from[swap.u], from[swap.v]);
    }

    ASSERT_EQ(from, to);
}

TEST(ApproxTSFinderTests, NoSwapTest) {
    {
        Assign from { 0, 1, 2, 3, 4 };
        Assign to   { 0, 1, 2, 3, 4 };
        CheckSwapSeq(from, to);
    }

    {
        Assign from { 4, 3, 2, 1, 0 };
        Assign to   { 4, 3, 2, 1, 0 };
        CheckSwapSeq(from, to);
    }
}

TEST(ApproxTSFinderTests, SimpleSwapTest) {
    {
        Assign from { 0, 1, 2, 4, 3 };
        Assign to   { 0, 1, 2, 3, 4 };
        CheckSwapSeq(from, to);
    }

    {
        Assign from { 0, 1, 2, 3, 4 };
        Assign to   { 0, 1, 2, 4, 3 };
        CheckSwapSeq(from, to);
    }

    {
        Assign from { 4, 3, 0, 1, 2 };
        Assign to   { 4, 3, 2, 1, 0 };
        CheckSwapSeq(from, to);
    }

    {
        Assign from { 4, 3, 2, 1, 0 };
        Assign to   { 4, 3, 0, 1, 2 };
        CheckSwapSeq(from, to);
    }
}

TEST(ApproxTSFinderTests, ComplexSwapTest) {
    {
        Assign from { 0, 1, 2, 4, 3 };
        Assign to   { 4, 3, 2, 1, 0 };
        CheckSwapSeq(from, to);
    }

    {
        Assign from { 4, 3, 0, 1, 2 };
        Assign to   { 0, 1, 2, 4, 3 };
        CheckSwapSeq(from, to);
    }

    {
        Assign from { 3, 4, 2, 0, 1 };
        Assign to   { 0, 1, 2, 3, 4 };
        CheckSwapSeq(from, to);
    }

    {
        Assign from { 3, 1, 2, 0, 4 };
        Assign to   { 0, 1, 2, 3, 4 };
        CheckSwapSeq(from, to);
    }
}

TEST(ApproxTSFinderTests, AllPermutationsTest) {
    std::vector<Assign> assigns;
    Assign perm { 0, 1, 2, 3, 4 };

    do {
        assigns.push_back(perm);
    } while (std::next_permutation(perm.begin(), perm.end()));

    ASSERT_EQ(assigns.size(), 120);

    for (uint32_t i = 0, e = assigns.size(); i < e; ++i)
        for (uint32_t j = 0; j < e; ++j)
            CheckSwapSeq(assigns[i], assigns[j]);
}
