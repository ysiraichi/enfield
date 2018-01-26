
#include "gtest/gtest.h"

#include "enfield/Support/CommandLine.h"

#include <vector>
#include <string>
#include <sstream>

#define CREATE_ARGS(nArgs, ...)                         \
    std::vector<std::string> argsStr = { __VA_ARGS__ }; \
    const char **argv = new const char*[nArgs];         \
    for (int i = 0; i < nArgs; ++i)                     \
        argv[i] = argsStr[i].c_str();


TEST(CommandLineTest, SimpleParammeterTest) {
    efd::Opt<bool> optBool("bool", "Some bool description.", false, false);
    efd::Opt<int> optInt("int", "Some int description.", false);
    efd::Opt<unsigned> optU("u", "Some unsigned description.", false);
    efd::Opt<long long> optLl("ll", "Some long long description.", false);
    efd::Opt<unsigned long long> optUll("ull", "Some unsigned long long description.", false);
    efd::Opt<float> optFloat("f", "Some flost description.", false);
    efd::Opt<double> optDouble("d", "Some double description.", false);
    efd::Opt<std::string> optStr("s", "Some string description.", false);

    int nArgs = 16;
    int constI = 10;
    std::string constS = "Enfield is comming!";

    CREATE_ARGS(nArgs, "SimpleParammeterTest",
        "-bool",
        "-int", std::to_string(constI),
        "-u",   std::to_string(constI) + "U",
        "-ll",  std::to_string(constI) + "L",
        "-ull", std::to_string(constI) + "L",
        "-f",   std::to_string(constI) + "f",
        "-d",   std::to_string(constI) + ".0",
        "-s",   constS,
    );

    efd::ParseArguments(nArgs, argv);

    EXPECT_TRUE(optBool.getVal());
    EXPECT_EQ(optInt.getVal(), constI);
    EXPECT_EQ(optU.getVal(), (unsigned) constI);
    EXPECT_EQ(optLl.getVal(), (long long) constI);
    EXPECT_EQ(optUll.getVal(), (unsigned long long) constI);
    EXPECT_EQ(optFloat.getVal(), (float) constI);
    EXPECT_EQ(optDouble.getVal(), (double) constI);
    EXPECT_EQ(optStr.getVal(), constS);
}

TEST(CommandLineTest, RequiredAssertTest) {
    efd::Opt<bool> optBool("bool", "Some bool description.", false, true);
    efd::Opt<int> optInt("int", "Some int description.", false);

    int nArgs = 3;

    CREATE_ARGS(nArgs, "RequiredAssertTest",
            "-int", "0");
    ASSERT_EXIT({ efd::ParseArguments(nArgs, argv); }, ::testing::ExitedWithCode(0), "");
}

TEST(CommandLineTest, NotEnoughArgumentsTest) {
    efd::Opt<bool> optBool("bool", "Some bool description.", false, true);
    efd::Opt<int> optInt("int", "Some int description.", false);

    int nArgs = 3;

    CREATE_ARGS(nArgs, "RequiredAssertTest",
            "-bool", "-int");
    ASSERT_DEATH({ efd::ParseArguments(nArgs, argv); }, 
            "Not enough command line arguments.");
}

namespace efd {
    template <> std::string Opt<std::vector<int>>::getStringVal() {
        std::string str;
        for (auto i : mVal) str += std::to_string(i) + "; ";
        return str;
    }
}

TEST(CommandLineTest, NoParserImplementedTest) {
    efd::Opt<std::vector<bool>> optVec("vec", "Passing a vector as argument.");

    int nArgs = 3;

    CREATE_ARGS(nArgs, "RequiredAssertTest",
            "-vec", "true false false true");
    ASSERT_DEATH({ efd::ParseArguments(nArgs, argv); }, 
            "Option with 'parse' function not implemented.");
}

namespace efd {
    template <> std::string Opt<std::vector<bool>>::getStringVal() {
        std::string str;
        for (auto i : mVal)
            if (i) str += "true; ";
            else str += "false; ";
        return str;
    }

    template <> void Opt<std::vector<int>>::parseImpl(std::vector<std::string> args) {
        std::string sVector = args[0];
        std::istringstream iVector(sVector);

        for (std::string val; iVector >> val;)
            mVal.push_back(std::stoi(val));
    }
}

TEST(CommandLineTest, ParserImplementationTest) {
    efd::Opt<std::vector<int>> optVec("vec", "Passing a vector as argument.");

    int nArgs = 3;
    std::vector<int> baseVal = { 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0 };

    std::string baseStr(" ");
    for (int i = 0, e = baseVal.size(); i < e; ++i)
        baseStr += std::to_string(baseVal[i]) + " ";


    CREATE_ARGS(nArgs, "RequiredAssertTest",
            "-vec", baseStr);

    efd::ParseArguments(nArgs, argv);

    const std::vector<int>& val = optVec.getVal();
    for (int i = 0, e = val.size(); i < e; ++i)
        ASSERT_EQ(val[i], baseVal[i]);
}
