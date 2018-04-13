#include "gtest/gtest.h"

#include "enfield/Transform/CircuitGraph.h"
#include "enfield/Support/Defs.h"

#include <string>

using namespace efd;

typedef Xbit::Type Type;

void FullTest(uint32_t qubits, uint32_t cbits, std::vector<std::vector<Xbit>> gates) {
    CircuitGraph ckt(qubits, cbits);

    for (uint32_t i = 0, e = gates.size(); i < e; ++i) {
        ckt.append(gates[i], reinterpret_cast<Node::Ref>(i));
    }

    auto it = ckt.build_iterator();

    for (uint32_t i = 0, e = gates.size(); i < e; ++i) {
        for (auto xbit : gates[i]) {
            ASSERT_TRUE(it.next(xbit));
            ASSERT_EQ(it.get(xbit), reinterpret_cast<Node::Ref>(i));
        }
    }

    for (uint32_t i = 0; i < qubits; ++i) {
        ASSERT_TRUE(it.next(Xbit::Q(i)));
        ASSERT_TRUE(it[Xbit::Q(i)]->isOutputNode());
    }

    for (uint32_t i = 0; i < cbits; ++i) {
        ASSERT_TRUE(it.next(Xbit::C(i)));
        ASSERT_TRUE(it[Xbit::C(i)]->isOutputNode());
    }
}

TEST(CircuitGraphTests, ClassicalBitsOnly) {
    FullTest(0, 5, {
            { Xbit::C(0), Xbit::C(1) },
            });

    FullTest(0, 5, {
            { Xbit::C(0), Xbit::C(1) },
            { Xbit::C(0), Xbit::C(1) },
            { Xbit::C(0), Xbit::C(1) },
            { Xbit::C(0), Xbit::C(1) },
            });

    FullTest(0, 5, {
            { Xbit::C(0), Xbit::C(1) },
            { Xbit::C(2), Xbit::C(3) },
            });

    FullTest(0, 5, {
            { Xbit::C(0), Xbit::C(1) },
            { Xbit::C(1), Xbit::C(2) },
            { Xbit::C(2), Xbit::C(3) },
            { Xbit::C(3), Xbit::C(4) },
            });

    FullTest(0, 5, {
            { Xbit::C(0), Xbit::C(1), Xbit::C(2), Xbit::C(3) },
            { Xbit::C(0), Xbit::C(1), Xbit::C(2), Xbit::C(4) },
            { Xbit::C(0), Xbit::C(1), Xbit::C(2), Xbit::C(3) },
            { Xbit::C(0), Xbit::C(1), Xbit::C(2), Xbit::C(4) },
            });
}

TEST(CircuitGraphTests, QuantumBitsOnly) {
    FullTest(5, 0, {
            { Xbit::Q(0), Xbit::Q(1) },
            });

    FullTest(5, 0, {
            { Xbit::Q(0), Xbit::Q(1) },
            { Xbit::Q(0), Xbit::Q(1) },
            { Xbit::Q(0), Xbit::Q(1) },
            { Xbit::Q(0), Xbit::Q(1) },
            });

    FullTest(5, 0, {
            { Xbit::Q(0), Xbit::Q(1) },
            { Xbit::Q(2), Xbit::Q(3) },
            });

    FullTest(5, 0, {
            { Xbit::Q(0), Xbit::Q(1) },
            { Xbit::Q(1), Xbit::Q(2) },
            { Xbit::Q(2), Xbit::Q(3) },
            { Xbit::Q(3), Xbit::Q(4) },
            });

    FullTest(5, 0, {
            { Xbit::Q(0), Xbit::Q(1), Xbit::Q(2), Xbit::Q(3) },
            { Xbit::Q(0), Xbit::Q(1), Xbit::Q(2), Xbit::Q(4) },
            { Xbit::Q(0), Xbit::Q(1), Xbit::Q(2), Xbit::Q(3) },
            { Xbit::Q(0), Xbit::Q(1), Xbit::Q(2), Xbit::Q(4) },
            });
}

TEST(CircuitGraphTests, ClassicalQuantumBits) {
    FullTest(5, 5, {
            { Xbit::Q(0), Xbit::C(1) },
            });

    FullTest(5, 5, {
            { Xbit::Q(0), Xbit::C(1) },
            { Xbit::Q(0), Xbit::C(1) },
            { Xbit::Q(0), Xbit::C(1) },
            { Xbit::Q(0), Xbit::C(1) },
            });

    FullTest(5, 5, {
            { Xbit::Q(0), Xbit::C(1) },
            { Xbit::Q(2), Xbit::C(3) },
            });

    FullTest(5, 5, {
            { Xbit::Q(0), Xbit::C(1) },
            { Xbit::Q(1), Xbit::C(2) },
            { Xbit::Q(2), Xbit::C(3) },
            { Xbit::Q(3), Xbit::C(4) },
            });

    FullTest(5, 5, {
            { Xbit::Q(0), Xbit::C(1), Xbit::Q(2), Xbit::C(3) },
            { Xbit::Q(0), Xbit::C(1), Xbit::Q(2), Xbit::C(4) },
            { Xbit::Q(0), Xbit::C(1), Xbit::Q(2), Xbit::C(3) },
            { Xbit::Q(0), Xbit::C(1), Xbit::Q(2), Xbit::C(4) },
            });
}

TEST(CircuitGraphTests, ErrorTests) {
    uint32_t exitCode = static_cast<uint32_t>(ExitCode::EXIT_unreachable);

    ASSERT_EXIT({ CircuitGraph ckt; ckt.append({}, nullptr); },
                ::testing::ExitedWithCode(exitCode), "");

    ASSERT_EXIT({ CircuitGraph ckt; ckt.build_iterator(); },
                ::testing::ExitedWithCode(exitCode), "");

    ASSERT_EXIT({ FullTest(5, 5, { { Xbit::Q(0), Xbit::C(9) } }); },
                ::testing::ExitedWithCode(exitCode), "");

    ASSERT_EXIT({ FullTest(5, 5, { { Xbit::Q(1), Xbit::C(8) } }); },
                ::testing::ExitedWithCode(exitCode), "");

    ASSERT_EXIT({ FullTest(5, 5, { { Xbit::Q(2), Xbit::C(7) } }); },
                ::testing::ExitedWithCode(exitCode), "");

    ASSERT_EXIT({ FullTest(5, 5, { { Xbit::Q(3), Xbit::C(6) } }); },
                ::testing::ExitedWithCode(exitCode), "");

    ASSERT_EXIT({ FullTest(5, 5, { { Xbit::Q(4), Xbit::C(5) } }); },
                ::testing::ExitedWithCode(exitCode), "");

    ASSERT_EXIT({ FullTest(5, 5, { { Xbit::Q(5), Xbit::C(4) } }); },
                ::testing::ExitedWithCode(exitCode), "");

    ASSERT_EXIT({ FullTest(5, 5, { { Xbit::Q(6), Xbit::C(3) } }); },
                ::testing::ExitedWithCode(exitCode), "");

    ASSERT_EXIT({ FullTest(5, 5, { { Xbit::Q(7), Xbit::C(2) } }); },
                ::testing::ExitedWithCode(exitCode), "");

    ASSERT_EXIT({ FullTest(5, 5, { { Xbit::Q(8), Xbit::C(1) } }); },
                ::testing::ExitedWithCode(exitCode), "");

    ASSERT_EXIT({ FullTest(5, 5, { { Xbit::Q(9), Xbit::C(0) } }); },
                ::testing::ExitedWithCode(exitCode), "");
}
