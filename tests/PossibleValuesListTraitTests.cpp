
#include "gtest/gtest.h"

#include "enfield/Support/PossibleValuesListTrait.h"
#include "enfield/Support/CommandLine.h"

#include <string>

using namespace efd;

enum class E { A, B, C, D, E, F, G, H, I, J, K, L };
typedef efd::EnumString<E, E::A, E::L> EEnum;
template<> std::vector<std::string> EEnum::mStrVal {
    "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L"
};

TEST(PossibleValuesListTraitTests, OnlyTest) {
    // This one test just checks if everything works as expected.
    EXPECT_EQ(PossibleValuesListTrait<EEnum>::Get(), EEnum::StringList());
}
