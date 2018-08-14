
#include "gtest/gtest.h"

#include "enfield/Support/EnumString.h"
#include "enfield/Support/CommandLine.h"

#include <string>

using namespace efd;

#define TEST_INIT_T(Ty, Ety, Val) \
    {\
        EXPECT_DEATH(\
                {\
                    Ty ty = static_cast<Ety>(Val);\
                    std::cout << (uint32_t) ty.getValue() << std::endl;\
                }, "");\
    }

#define TEST_INIT_STR(Ty, Val) \
    {\
        EXPECT_DEATH(\
                {\
                    Ty ty = Val;\
                    std::cout << (uint32_t) ty.getValue() << std::endl;\
                }, "");\
    }

#define TEST_ENUM_FAIL(Ty, Ety, E) \
    {\
        Ty ty = Ety::E;\
        EXPECT_EQ(ty.getValue(), Ety::E);\
        EXPECT_DEATH({ std::cout << ty.getStringValue(); }, "");\
    }

#define TEST_ENUM(Ty, Ety, E) \
    {\
        Ty ty = Ety::E;\
        EXPECT_EQ(ty.getValue(), Ety::E);\
        EXPECT_EQ(ty.getStringValue(), #E);\
    }

enum A { AA, AB, AC, AD };
typedef EnumString<A, AA, AD> AEnum;

template<> std::vector<std::string> AEnum::mStrVal {
    "AA", "AB", "AC", "AD"
};

TEST(EnumStringTests, SimpleTest) {
    TEST_ENUM(AEnum, A, AA);
    TEST_ENUM(AEnum, A, AB);
    TEST_ENUM(AEnum, A, AC);
    TEST_ENUM(AEnum, A, AD);
}

enum class B { AA, AB, AC, AD };
typedef EnumString<B, B::AA, B::AD> BEnum;

template<> std::vector<std::string> BEnum::mStrVal {
    "AA", "AB", "AC", "AD"
};

TEST(EnumStringTests, EnumClassSimpleTest) {
    TEST_ENUM(BEnum, B, AA);
    TEST_ENUM(BEnum, B, AB);
    TEST_ENUM(BEnum, B, AC);
    TEST_ENUM(BEnum, B, AD);
}

enum class C { AA = 200, AB, AC, AD };
typedef EnumString<C, C::AA, C::AD> CEnum;

template<> std::vector<std::string> CEnum::mStrVal {
    "AA", "AB", "AC", "AD"
};

TEST(EnumStringTests, OffsetSimpleTest) {
    TEST_ENUM(CEnum, C, AA);
    TEST_ENUM(CEnum, C, AB);
    TEST_ENUM(CEnum, C, AC);
    TEST_ENUM(CEnum, C, AD);
}

TEST(EnumStringTests, InitTErrorsTest) {
    TEST_INIT_T(AEnum, A, 4);
    TEST_INIT_T(AEnum, A, 5);
    TEST_INIT_T(AEnum, A, 6);
    TEST_INIT_T(AEnum, A, 7);
    TEST_INIT_T(AEnum, A, 8);
    TEST_INIT_T(AEnum, A, 9);
    TEST_INIT_T(AEnum, A, 10);
    TEST_INIT_T(BEnum, B, 4);
    TEST_INIT_T(BEnum, B, 5);
    TEST_INIT_T(BEnum, B, 6);
    TEST_INIT_T(BEnum, B, 7);
    TEST_INIT_T(BEnum, B, 8);
    TEST_INIT_T(BEnum, B, 9);
    TEST_INIT_T(BEnum, B, 10);
    TEST_INIT_T(CEnum, C, 4);
    TEST_INIT_T(CEnum, C, 5);
    TEST_INIT_T(CEnum, C, 6);
    TEST_INIT_T(CEnum, C, 7);
    TEST_INIT_T(CEnum, C, 8);
    TEST_INIT_T(CEnum, C, 9);
    TEST_INIT_T(CEnum, C, 10);
}

TEST(EnumStringTests, InitSTRErrorsTest) {
    TEST_INIT_STR(AEnum, "AAA");
    TEST_INIT_STR(AEnum, "ABA");
    TEST_INIT_STR(AEnum, "ACA");
    TEST_INIT_STR(AEnum, "ADA");
    TEST_INIT_STR(AEnum, "AAB");
    TEST_INIT_STR(AEnum, "AAC");
    TEST_INIT_STR(AEnum, "AAD");
    TEST_INIT_STR(BEnum, "AAA");
    TEST_INIT_STR(BEnum, "ABA");
    TEST_INIT_STR(BEnum, "ACA");
    TEST_INIT_STR(BEnum, "ADA");
    TEST_INIT_STR(BEnum, "AAB");
    TEST_INIT_STR(BEnum, "AAC");
    TEST_INIT_STR(BEnum, "AAD");
    TEST_INIT_STR(CEnum, "AAA");
    TEST_INIT_STR(CEnum, "ABA");
    TEST_INIT_STR(CEnum, "ACA");
    TEST_INIT_STR(CEnum, "ADA");
    TEST_INIT_STR(CEnum, "AAB");
    TEST_INIT_STR(CEnum, "AAC");
    TEST_INIT_STR(CEnum, "AAD");
}

enum class E { A = 0, B, C, D = 200, E, F, G, H, I = 8, J, K, L = 300 };
typedef EnumString<E, E::A, E::L> EEnum;

template<> std::vector<std::string> EEnum::mStrVal {
    "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L"
};


TEST(EnumStringTests, EnumErrorsTest) {
    TEST_ENUM(EEnum, E, A);
    TEST_ENUM(EEnum, E, B);
    TEST_ENUM(EEnum, E, C);
    TEST_ENUM_FAIL(EEnum, E, D);
    TEST_ENUM_FAIL(EEnum, E, E);
    TEST_ENUM_FAIL(EEnum, E, F);
    TEST_ENUM_FAIL(EEnum, E, G);
    TEST_ENUM_FAIL(EEnum, E, H);
    TEST_ENUM(EEnum, E, I);
    TEST_ENUM(EEnum, E, J);
    TEST_ENUM(EEnum, E, K);
    TEST_ENUM_FAIL(EEnum, E, L);
}
