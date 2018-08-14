
#include "gtest/gtest.h"

#include "enfield/Support/CommandLine.h"
#include "enfield/Support/Defs.h"

#include <vector>
#include <string>
#include <sstream>

using namespace efd;

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

    CREATE_ARGS(nArgs, "RequiredAssertTest", "-int", "0");
    ASSERT_EXIT({ efd::ParseArguments(nArgs, argv); },
            ::testing::ExitedWithCode(0), "");
}

TEST(CommandLineTest, NotEnoughArgumentsTest) {
    efd::Opt<bool> optBool("bool", "Some bool description.", false, true);
    efd::Opt<int> optInt("int", "Some int description.", false);

    int nArgs = 3;

    CREATE_ARGS(nArgs, "RequiredAssertTest", "-bool", "-int");
    ASSERT_DEATH({ efd::ParseArguments(nArgs, argv); }, "");
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

    CREATE_ARGS(nArgs, "RequiredAssertTest", "-vec", "true false false true");
    ASSERT_DEATH({ efd::ParseArguments(nArgs, argv); }, "");
}

namespace efd {
    template <> std::string Opt<std::vector<bool>>::getStringVal() {
        std::string str;
        for (auto i : mVal)
            if (i) str += "true; ";
            else str += "false; ";
        return str;
    }

    template <> struct ParseOptTrait<std::vector<int>> {
        static void Run(Opt<std::vector<int>>* opt,
                        std::vector<std::string> args) {
            std::string sVector = args[0];
            std::istringstream iVector(sVector);
            for (std::string val; iVector >> val;)
                opt->mVal.push_back(std::stoi(val));
        }
    };
    
    template <> std::string Opt<std::map<std::string, uint32_t>>::getStringVal() {
        std::string str;
        for (auto& pair : mVal)
            str += pair.first + ":" + std::to_string(pair.second) + " ";
        return str;
    }

    template <> struct ParseOptTrait<std::map<std::string, uint32_t>> {
        static void Run(Opt<std::map<std::string, uint32_t>>* opt,
                        std::vector<std::string> args) {
            std::string sVector = args[0];
            std::istringstream iVector(sVector);

            for (std::string str; iVector >> str;) {
                std::size_t i = str.find(':');
                int32_t l = i, r = i;

                if (i != std::string::npos) {
                    while (l >= 0 && !isspace(str[l])) --l;
                    while (r < (int32_t) str.length() && !isspace(str[r])) ++r;
                    ++l; --r;
                }

                std::string key = str.substr(l, i - l);
                std::string val = str.substr(i + 1, r - i);
                opt->mVal[key] = std::stoull(val);
            }
        }
    };
}

TEST(CommandLineTest, ParserImplementationTest) {
    {
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
    {
        efd::Opt<std::map<std::string, uint32_t>>
            optMap("map", "Passing a map as argument.");

        int nArgs = 3;
        std::map<std::string, uint32_t> baseVal {
            {"h", 20}, {"hh", 40},{"hhh", 80},{"hhhh", 160}
        };

        std::string baseStr;
        for (auto& pair : baseVal)
            baseStr += pair.first + ":" + std::to_string(pair.second) + " ";


        CREATE_ARGS(nArgs, "RequiredAssertTest",
                "-map", baseStr);

        efd::ParseArguments(nArgs, argv);

        const std::map<std::string, uint32_t>& val = optMap.getVal();
        for (auto& pair : val) {
            ASSERT_TRUE(baseVal.find(pair.first) != baseVal.end());
            ASSERT_EQ(pair.second, baseVal[pair.first]);
        }
    }
}
