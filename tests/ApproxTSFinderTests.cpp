#include "gtest/gtest.h"
#include "enfield/Support/ApproxTSFinder.h"
#include "enfield/Support/ExpTSFinder.h"
#include "enfield/Support/Timer.h"

using namespace efd;

static ExpTSFinder::uRef expfinder;
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
        expfinder = ExpTSFinder::Create();
        expfinder->setGraph(graph.get());
    }

    return graph;
}

static void CheckSwapSeq(Graph::sRef graph, Assign from, Assign to, bool usingundef) {
    Timer::Start();
    ApproxTSFinder approxfinder;
    approxfinder.setGraph(graph.get());

    auto approxseq = approxfinder.find(from, to);

    if (!usingundef) {
        auto expseq = expfinder->find(from, to);

        ASSERT_TRUE(approxseq.size() <= 4 * expseq.size());

        if (approxseq.size() != expseq.size()) {
            std::cerr << "4-Approx: " << approxseq.size() << std::endl;
            std::cerr << "Exp: " << expseq.size() << std::endl;
        }
    } else {
        std::cerr << "4-Approx: " << approxseq.size() << std::endl;
    }

    if (!approxseq.empty()) {
        for (auto swap : approxseq)
            std::swap(from[swap.u], from[swap.v]);
    }

    ASSERT_EQ(from, to);
    std::cerr << "time: " << Timer::Stop<std::chrono::microseconds>() << std::endl;
}

static void CheckSwapSeqWrapper(Assign from, Assign to, bool usingundef = false) {
    CheckSwapSeq(GetGraph(), from, to, usingundef);
}

TEST(ApproxTSFinderTests, NoSwapTest) {
    {
        Assign from { 0, 1, 2, 3, 4 };
        Assign to   { 0, 1, 2, 3, 4 };
        CheckSwapSeqWrapper(from, to);
    }

    {
        Assign from { 4, 3, 2, 1, 0 };
        Assign to   { 4, 3, 2, 1, 0 };
        CheckSwapSeqWrapper(from, to);
    }
}

TEST(ApproxTSFinderTests, SimpleSwapTest) {
    {
        Assign from { 0, 1, 2, 4, 3 };
        Assign to   { 0, 1, 2, 3, 4 };
        CheckSwapSeqWrapper(from, to);
    }

    {
        Assign from { 0, 1, 2, 3, 4 };
        Assign to   { 0, 1, 2, 4, 3 };
        CheckSwapSeqWrapper(from, to);
    }

    {
        Assign from { 4, 3, 0, 1, 2 };
        Assign to   { 4, 3, 2, 1, 0 };
        CheckSwapSeqWrapper(from, to);
    }

    {
        Assign from { 4, 3, 2, 1, 0 };
        Assign to   { 4, 3, 0, 1, 2 };
        CheckSwapSeqWrapper(from, to);
    }
}

TEST(ApproxTSFinderTests, ComplexSwapTest) {
    {
        Assign from { 0, 1, 2, 4, 3 };
        Assign to   { 4, 3, 2, 1, 0 };
        CheckSwapSeqWrapper(from, to);
    }

    {
        Assign from { 4, 3, 0, 1, 2 };
        Assign to   { 0, 1, 2, 4, 3 };
        CheckSwapSeqWrapper(from, to);
    }

    {
        Assign from { 3, 4, 2, 0, 1 };
        Assign to   { 0, 1, 2, 3, 4 };
        CheckSwapSeqWrapper(from, to);
    }

    {
        Assign from { 3, 1, 2, 0, 4 };
        Assign to   { 0, 1, 2, 3, 4 };
        CheckSwapSeqWrapper(from, to);
    }
}

TEST(ApproxTSFinderTests, AllPermutationsTest) {
    std::vector<Assign> assigns;
    Assign perm { 0, 1, 2, 3, 4 };

    do {
        assigns.push_back(perm);
    } while (std::next_permutation(perm.begin(), perm.end()));

    ASSERT_EQ(assigns.size(), (uint32_t) 120);

    uint32_t x = 0;
    for (uint32_t i = 0, e = assigns.size(); i < e; ++i)
        for (uint32_t j = 0; j < e; ++j, ++x)
            CheckSwapSeqWrapper(assigns[i], assigns[j]);
}

// Using undef
TEST(ApproxTSFinderTests, UndefNoSwapTest) {
    {
        Assign from { 0, _undef, 2, 4, _undef };
        Assign to   { 0, _undef, 2, 4, _undef };
        CheckSwapSeqWrapper(from, to, true);
    }

    {
        Assign from { _undef, _undef, _undef, _undef, _undef };
        Assign to   { _undef, _undef, _undef, _undef, _undef };
        CheckSwapSeqWrapper(from, to, true);
    }
}

TEST(ApproxTSFinderTests, UndefOneSwapTest) {
    {
        Assign from { 0, _undef, 2, 4, _undef };
        Assign to   { 0, _undef, 4, 2, _undef };
        CheckSwapSeqWrapper(from, to, true);
    }

    {
        Assign from { 1, _undef, 0, _undef, _undef };
        Assign to   { 0, _undef, 1, _undef, _undef };
        CheckSwapSeqWrapper(from, to, true);
    }
}

TEST(ApproxTSFinderTests, UndefComplexSwapTest) {
    {
        Assign from { 0, _undef, 2, 4, _undef };
        Assign to   { _undef, _undef, 4, 2, 0 };
        CheckSwapSeqWrapper(from, to, true);
    }

    {
        Assign from { 1, _undef, 0, _undef, _undef };
        Assign to   { _undef, _undef, 1, _undef, 0 };
        CheckSwapSeqWrapper(from, to, true);
    }

    {
        Assign from { 1, _undef, 0, 4, 3 };
        Assign to   { 4, 1, 0, 3, _undef };
        CheckSwapSeqWrapper(from, to, true);
    }
}

TEST(ApproxTSFinderTests, Undef16QComplexSwapTest) {
    Graph::sRef graph;

    const std::string graphstr =
"\
16\n\
0 1\n\
1 2\n\
2 3\n\
3 14\n\
4 3\n\
4 5\n\
6 7\n\
6 11\n\
7 10\n\
8 7\n\
9 8\n\
9 10\n\
11 10\n\
12 5\n\
12 11\n\
12 13\n\
13 4\n\
13 14\n\
15 0\n\
15 14\
";
    
    graph = Graph::ReadString(graphstr);

    {
        Assign from {
            0, 1, 2, 3,
            4, 5, 6, 7,
            8, 9, 10, 11,
            12, 13, 14, 15
        };
        Assign to {
            0, 1, 2, 3,
            4, 5, 6, 7,
            8, 9, 10, 11,
            12, 13, 14, 15
        };
        CheckSwapSeq(graph, from, to, true);
    }

    {
        Assign from {
            0, 1, 2, 3,
            4, 12, 6, 7,
            8, 9, 10, 11,
            5, 13, 14, 15
        };
        Assign to {
            0, 1, 2, 3,
            4, 5, 6, 7,
            8, 9, 10, 11,
            12, 13, 14, 15
        };
        CheckSwapSeq(graph, from, to, true);
    }

    {
        Assign from {
            0, 1, 2, 3,
            4, 5, 6, 7,
            8, 9, 10, 11,
            12, 13, 14, 15
        };
        Assign to {
            3, 2, 1, 0,
            4, 5, 6, 7,
            8, 9, 10, 11,
            12, 13, 14, 15
        };
        CheckSwapSeq(graph, from, to, true);
    }

    {
        Assign from {
            0, 15, 1, 14,
            13, 3, 12, 4,
            _undef, _undef, _undef, _undef,
            _undef, _undef, _undef, _undef
        };
        Assign to {
            _undef, _undef, _undef, _undef,
            _undef, _undef, _undef, _undef,
            0, 15, 1, 14,
            13, 3, 12, 4
        };
        CheckSwapSeq(graph, from, to, true);
    }

    {
        Assign from {
            0, 15, 1, 14,
            13, 3, 12, 4,
            _undef, _undef, _undef, _undef,
            _undef, _undef, _undef, _undef
        };
        Assign to {
            _undef, _undef, _undef, _undef,
            _undef, _undef, _undef, _undef,
            4, 12, 3, 13,
            14, 1, 15, 0
        };
        CheckSwapSeq(graph, from, to, true);
    }

    {
        Assign from {
            0, 1, 2, 3,
            4, 5, 6, 7,
            _undef, _undef, _undef, _undef,
            _undef, _undef, _undef, _undef
        };
        Assign to {
            _undef, _undef, _undef, _undef,
            _undef, _undef, _undef, _undef,
            7, 5, 4, 6,
            1, 3, 2, 0
        };
        CheckSwapSeq(graph, from, to, true);
    }
}
