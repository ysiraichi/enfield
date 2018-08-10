#include "gtest/gtest.h"

#include "enfield/Analysis/Driver.h"
#include "enfield/Analysis/NodeVisitor.h"
#include "enfield/Transform/QModule.h"
#include "enfield/Transform/DependencyBuilderPass.h"
#include "enfield/Transform/PassCache.h"
#include "enfield/Support/RTTI.h"
#include "enfield/Support/uRefCast.h"

#include <string>
#include <unordered_map>

using namespace efd;

TEST(DependencyBuilderWrapperPassTest, GateDependenciesTest) {
    {
        const std::string program = \
"\
gate cnot x, y {\
    CX x, y;\
}\
";

        auto qmod = toShared(QModule::ParseString(program));
        auto pass = DependencyBuilderWrapperPass::Create();
        pass->run(qmod.get());

        uint32_t x = 0;
        uint32_t y = 1;

        auto sign = qmod->getQGate("cnot");
        ASSERT_FALSE(sign == nullptr);
        NDGateDecl::Ref gate = dynCast<NDGateDecl>(sign);
        ASSERT_FALSE(gate == nullptr);

        auto data = pass->getData();
        auto deps = data.getDependencies(gate);
        // Has only one parallel dependency.
        ASSERT_EQ(deps.size(), (uint32_t) 1);
        // The only paralel dependency has only one parallel dependency.
        ASSERT_EQ(deps[0].size(), (uint32_t) 1);
        ASSERT_EQ(deps[0][0].mFrom, x);
        ASSERT_EQ(deps[0][0].mTo, y);

        ASSERT_FALSE(deps[0].mCallPoint == nullptr);
        ASSERT_TRUE(efd::instanceOf<NDQOpCX>(deps[0].mCallPoint));

        PassCache::Clear();
    }

    {
        const std::string program = \
"\
gate cx x, y {\
    CX x, y;\
}\
gate cnot x, y {\
    cx x, y;\
}\
";

        auto qmod = toShared(QModule::ParseString(program));
        auto pass = DependencyBuilderWrapperPass::Create();
        pass->run(qmod.get());

        uint32_t x = 0;
        uint32_t y = 1;

        auto cxSign = qmod->getQGate("cx");
        ASSERT_FALSE(cxSign == nullptr);
        NDGateDecl::Ref cxGate = dynCast<NDGateDecl>(cxSign);
        ASSERT_FALSE(cxGate == nullptr);

        auto cnotSign = qmod->getQGate("cnot");
        ASSERT_FALSE(cnotSign == nullptr);
        NDGateDecl::Ref cnotGate = dynCast<NDGateDecl>(cnotSign);
        ASSERT_FALSE(cnotGate == nullptr);

        auto data = pass->getData();
        auto cxDeps = data.getDependencies(cxGate);
        auto cnotDeps = data.getDependencies(cnotGate);

        // Has only one parallel dependency.
        ASSERT_EQ(cxDeps.size(), (uint32_t) 1);
        // The only parallel dependency has only one parallel dependency.
        ASSERT_EQ(cxDeps[0].size(), (uint32_t) 1);
        ASSERT_EQ(cxDeps[0][0].mFrom, x);
        ASSERT_EQ(cxDeps[0][0].mTo, y);

        ASSERT_FALSE(cxDeps[0].mCallPoint == nullptr);
        ASSERT_TRUE(efd::instanceOf<NDQOpCX>(cxDeps[0].mCallPoint));

        // Has only one parallel dependency.
        ASSERT_EQ(cnotDeps.size(), (uint32_t) 1);
        // The only parallel dependency has only one parallel dependency.
        ASSERT_EQ(cnotDeps[0].size(), (uint32_t) 1);
        ASSERT_EQ(cnotDeps[0][0].mFrom, x);
        ASSERT_EQ(cnotDeps[0][0].mTo, y);

        ASSERT_FALSE(cnotDeps[0].mCallPoint == nullptr);
        ASSERT_TRUE(efd::instanceOf<NDQOp>(cnotDeps[0].mCallPoint));

        PassCache::Clear();
    }
}

TEST(DependencyBuilderWrapperPassTest, ProgramDependenciesTest) {
    {
        const std::string program = \
"\
qreg q[2];\
CX q[0], q[1];\
";

        auto qmod = toShared(QModule::ParseString(program));
        auto pass = DependencyBuilderWrapperPass::Create();
        pass->run(qmod.get());

        uint32_t q0 = 0;
        uint32_t q1 = 1;

        auto data = pass->getData();
        auto deps = data.getDependencies();

        // Has only one parallel dependency.
        ASSERT_EQ(deps.size(), (uint32_t) 1);
        // The only parallel dependency has only one parallel dependency.
        ASSERT_EQ(deps[0].size(), (uint32_t) 1);
        ASSERT_EQ(deps[0][0].mFrom, q0);
        ASSERT_EQ(deps[0][0].mTo, q1);

        ASSERT_FALSE(deps[0].mCallPoint == nullptr);
        ASSERT_TRUE(efd::instanceOf<NDQOpCX>(deps[0].mCallPoint));

        PassCache::Clear();
    }

    {
        const std::string program = \
"\
include \"qelib1.inc\";\
gate majority a, b, c {\
cx c, b;\
cx c, a;\
ccx a, b, c;\
}\
gate unmaj a, b, c {\
ccx a, b, c;\
cx c, a;\
cx a, b;\
}\
gate add4 a0, a1, a2, a3, b0, b1, b2, b3, cin, cout {\
majority cin, b0, a0;\
majority a0, b1, a1;\
majority a1, b2, a2;\
majority a2, b3, a3;\
cx a3, cout;\
unmaj a2, b3, a3;\
unmaj a1, b2, a2;\
unmaj a0, b1, a1;\
unmaj cin, b0, a0;\
}\
qreg carry[2];\
qreg a[8];\
qreg b[8];\
creg ans[8];\
creg carryout[1];\
x a[0];\
x b[0];\
x b[1];\
x b[2];\
x b[3];\
x b[4];\
x b[5];\
x b[6];\
x b[7];\
add4 a[0], a[1], a[2], a[3], b[0], b[1], b[2], b[3], carry[0], carry[1];\
add4 a[4], a[5], a[6], a[7], b[4], b[5], b[6], b[7], carry[1], carry[0];\
measure b[0] -> ans[0];\
measure b[1] -> ans[1];\
measure b[2] -> ans[2];\
measure b[3] -> ans[3];\
measure b[4] -> ans[4];\
measure b[5] -> ans[5];\
measure b[6] -> ans[6];\
measure b[7] -> ans[7];\
measure carry[0] -> carryout[0];\
";

        auto qmod = toShared(QModule::ParseString(program));
        auto pass = DependencyBuilderWrapperPass::Create();
        pass->run(qmod.get());

        std::unordered_map<std::string, std::pair<uint32_t, uint32_t>> gatesInfo = {
            { "ccx", { 6, 6 } }, { "cx", { 1, 1 } }, { "x", { 0, 0 } }, { "majority", { 3, 8 } }, { "add4", { 9, 65 } }, { "unmaj", { 3, 8 } }
        };

        auto data = pass->getData();
        DependencyBuilder::DepsVector deps;
        for (auto pair : gatesInfo) {
            auto sign = qmod->getQGate(pair.first);
            NDGateDecl::Ref gate = dynCast<NDGateDecl>(sign);
            ASSERT_FALSE(gate == nullptr);

            auto deps = data.getDependencies(gate);
            ASSERT_EQ(deps.size(), pair.second.first);

            uint32_t sum = 0;
            for (auto v : deps) sum += v.size();
            ASSERT_EQ(sum, pair.second.second);
        }

        PassCache::Clear();
    }
}
