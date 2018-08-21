#include "gtest/gtest.h"
#include "enfield/Support/SimplifiedApproxTSFinder.h"
#include "enfield/Support/ExpTSFinder.h"
#include "enfield/Support/Timer.h"

using namespace efd;

static ExpTSFinder::uRef expFinder;
static SimplifiedApproxTSFinder::uRef approxFinder5V;
static SimplifiedApproxTSFinder::uRef approxFinder16V;

static Graph::sRef graph5V;
static Graph::sRef graph16V;

static const std::string graph5VStr =
"{\
    \"vertices\": 5,\
    \"type\": \"Undirected\",\
    \"adj\": [\
        [ {\"v\": 1}, {\"v\": 2} ],\
        [ {\"v\": 2} ],\
        [],\
        [ {\"v\": 2}, {\"v\": 4} ],\
        [ {\"v\": 2} ]\
    ]\
}";
static const std::string graph16VStr =
"{\
    \"vertices\": 16,\
    \"type\": \"Undirected\",\
    \"adj\": [\
        [ {\"v\": 1} ],\
        [ {\"v\": 2} ],\
        [ {\"v\": 3} ],\
        [ {\"v\": 14} ],\
        [ {\"v\": 3}, {\"v\": 5} ],\
        [],\
        [ {\"v\": 7}, {\"v\": 11} ],\
        [ {\"v\": 10} ],\
        [ {\"v\": 7} ],\
        [ {\"v\": 8}, {\"v\": 10} ],\
        [],\
        [ {\"v\": 10} ],\
        [ {\"v\": 5}, {\"v\": 11}, {\"v\": 13} ],\
        [ {\"v\": 4}, {\"v\": 14} ],\
        [],\
        [ {\"v\": 0}, {\"v\": 14} ]\
    ]\
}";

struct TestEnvironment : public ::testing::Environment {
    void SetUp() override {
        graph5V = JsonParser<efd::Graph>::ParseString(graph5VStr);
        graph16V = JsonParser<efd::Graph>::ParseString(graph16VStr);

        expFinder = ExpTSFinder::Create();
        approxFinder5V = SimplifiedApproxTSFinder::Create();
        approxFinder16V = SimplifiedApproxTSFinder::Create();

        expFinder->setGraph(graph5V.get());
        approxFinder5V->setGraph(graph5V.get());
        approxFinder16V->setGraph(graph16V.get());
    }
};

::testing::Environment* const env =
::testing::AddGlobalTestEnvironment(new TestEnvironment());

static void CheckSwapSeq(SimplifiedApproxTSFinder::Ref approx,
                         const InverseMap& from,
                         const InverseMap& to,
                         bool hasUndef) {
    Timer::Start();

    auto approxseq = approx->find(from, to);

    if (!hasUndef) {
        auto expseq = expFinder->find(from, to);

        ASSERT_TRUE(approxseq.size() <= 4 * expseq.size());

        if (approxseq.size() != expseq.size()) {
            std::cerr << "4-Approx: " << approxseq.size() << std::endl;
            std::cerr << "Exp: " << expseq.size() << std::endl;
        }
    } else {
        std::cerr << "4-Approx: " << approxseq.size() << std::endl;
    }

    auto _from = from;
    if (!approxseq.empty()) {
        for (auto swap : approxseq)
            std::swap(_from[swap.u], _from[swap.v]);
    }

    ASSERT_EQ(_from, to);
    std::cerr << "time: " << Timer::Stop<std::chrono::microseconds>() << std::endl;
}

static void CheckSwapSeq5V(const InverseMap& from, const InverseMap& to, bool hasUndef = false) {
    CheckSwapSeq(approxFinder5V.get(), from, to, hasUndef);
}

static void CheckSwapSeq16V(const InverseMap& from, const InverseMap& to, bool hasUndef = true) {
    CheckSwapSeq(approxFinder16V.get(), from, to, hasUndef);
}

TEST(ApproxTSFinderTests, NoSwapTest) {
    {
        InverseMap from { 0, 1, 2, 3, 4 };
        InverseMap to   { 0, 1, 2, 3, 4 };
        CheckSwapSeq5V(from, to);
    }

    {
        InverseMap from { 4, 3, 2, 1, 0 };
        InverseMap to   { 4, 3, 2, 1, 0 };
        CheckSwapSeq5V(from, to);
    }
}

TEST(ApproxTSFinderTests, SimpleSwapTest) {
    {
        InverseMap from { 0, 1, 2, 4, 3 };
        InverseMap to   { 0, 1, 2, 3, 4 };
        CheckSwapSeq5V(from, to);
    }

    {
        InverseMap from { 0, 1, 2, 3, 4 };
        InverseMap to   { 0, 1, 2, 4, 3 };
        CheckSwapSeq5V(from, to);
    }

    {
        InverseMap from { 4, 3, 0, 1, 2 };
        InverseMap to   { 4, 3, 2, 1, 0 };
        CheckSwapSeq5V(from, to);
    }

    {
        InverseMap from { 4, 3, 2, 1, 0 };
        InverseMap to   { 4, 3, 0, 1, 2 };
        CheckSwapSeq5V(from, to);
    }
}

TEST(ApproxTSFinderTests, ComplexSwapTest) {
    {
        InverseMap from { 0, 1, 2, 4, 3 };
        InverseMap to   { 4, 3, 2, 1, 0 };
        CheckSwapSeq5V(from, to);
    }

    {
        InverseMap from { 4, 3, 0, 1, 2 };
        InverseMap to   { 0, 1, 2, 4, 3 };
        CheckSwapSeq5V(from, to);
    }

    {
        InverseMap from { 3, 4, 2, 0, 1 };
        InverseMap to   { 0, 1, 2, 3, 4 };
        CheckSwapSeq5V(from, to);
    }

    {
        InverseMap from { 3, 1, 2, 0, 4 };
        InverseMap to   { 0, 1, 2, 3, 4 };
        CheckSwapSeq5V(from, to);
    }
}

TEST(ApproxTSFinderTests, AllPermutationsTest) {
    std::vector<InverseMap> invs;
    InverseMap perm { 0, 1, 2, 3, 4 };

    do {
        invs.push_back(perm);
    } while (std::next_permutation(perm.begin(), perm.end()));

    ASSERT_EQ(invs.size(), (uint32_t) 120);

    uint32_t x = 0;
    for (uint32_t i = 0, e = invs.size(); i < e; ++i)
        for (uint32_t j = 0; j < e; ++j, ++x)
            CheckSwapSeq5V(invs[i], invs[j]);
}

// Using undef
TEST(ApproxTSFinderTests, UndefNoSwapTest) {
    {
        InverseMap from { 0, _undef, 2, 4, _undef };
        InverseMap to   { 0, _undef, 2, 4, _undef };
        CheckSwapSeq5V(from, to, true);
    }

    {
        InverseMap from { _undef, _undef, _undef, _undef, _undef };
        InverseMap to   { _undef, _undef, _undef, _undef, _undef };
        CheckSwapSeq5V(from, to, true);
    }
}

TEST(ApproxTSFinderTests, UndefOneSwapTest) {
    {
        InverseMap from { 0, _undef, 2, 4, _undef };
        InverseMap to   { 0, _undef, 4, 2, _undef };
        CheckSwapSeq5V(from, to, true);
    }

    {
        InverseMap from { 1, _undef, 0, _undef, _undef };
        InverseMap to   { 0, _undef, 1, _undef, _undef };
        CheckSwapSeq5V(from, to, true);
    }
}

TEST(ApproxTSFinderTests, UndefComplexSwapTest) {
    {
        InverseMap from { 0, _undef, 2, 4, _undef };
        InverseMap to   { _undef, _undef, 4, 2, 0 };
        CheckSwapSeq5V(from, to, true);
    }

    {
        InverseMap from { 1, _undef, 0, _undef, _undef };
        InverseMap to   { _undef, _undef, 1, _undef, 0 };
        CheckSwapSeq5V(from, to, true);
    }

    {
        InverseMap from { 1, _undef, 0, 4, 3 };
        InverseMap to   { 4, 1, 0, 3, _undef };
        CheckSwapSeq5V(from, to, true);
    }
}

TEST(ApproxTSFinderTests, Undef16QComplexSwapTest) {
    {
        InverseMap from {
            0, 1, 2, 3,
            4, 5, 6, 7,
            8, 9, 10, 11,
            12, 13, 14, 15
        };
        InverseMap to {
            0, 1, 2, 3,
            4, 5, 6, 7,
            8, 9, 10, 11,
            12, 13, 14, 15
        };
        CheckSwapSeq16V(from, to, true);
    }

    {
        InverseMap from {
            0, 1, 2, 3,
            4, 12, 6, 7,
            8, 9, 10, 11,
            5, 13, 14, 15
        };
        InverseMap to {
            0, 1, 2, 3,
            4, 5, 6, 7,
            8, 9, 10, 11,
            12, 13, 14, 15
        };
        CheckSwapSeq16V(from, to, true);
    }

    {
        InverseMap from {
            0, 1, 2, 3,
            4, 5, 6, 7,
            8, 9, 10, 11,
            12, 13, 14, 15
        };
        InverseMap to {
            3, 2, 1, 0,
            4, 5, 6, 7,
            8, 9, 10, 11,
            12, 13, 14, 15
        };
        CheckSwapSeq16V(from, to, true);
    }

    {
        InverseMap from {
            0, 15, 1, 14,
            13, 3, 12, 4,
            _undef, _undef, _undef, _undef,
            _undef, _undef, _undef, _undef
        };
        InverseMap to {
            _undef, _undef, _undef, _undef,
            _undef, _undef, _undef, _undef,
            0, 15, 1, 14,
            13, 3, 12, 4
        };
        CheckSwapSeq16V(from, to, true);
    }

    {
        InverseMap from {
            0, 15, 1, 14,
            13, 3, 12, 4,
            _undef, _undef, _undef, _undef,
            _undef, _undef, _undef, _undef
        };
        InverseMap to {
            _undef, _undef, _undef, _undef,
            _undef, _undef, _undef, _undef,
            4, 12, 3, 13,
            14, 1, 15, 0
        };
        CheckSwapSeq16V(from, to, true);
    }

    {
        InverseMap from {
            0, 1, 2, 3,
            4, 5, 6, 7,
            _undef, _undef, _undef, _undef,
            _undef, _undef, _undef, _undef
        };
        InverseMap to {
            _undef, _undef, _undef, _undef,
            _undef, _undef, _undef, _undef,
            7, 5, 4, 6,
            1, 3, 2, 0
        };
        CheckSwapSeq16V(from, to, true);
    }
}
