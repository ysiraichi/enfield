
#include "gtest/gtest.h"

#include "enfield/Analysis/FlattenPass.h"
#include "enfield/Analysis/QModule.h"

#include <string>

using namespace efd;

TEST(FlattenPassTests, CXTest) {
    {
        const std::string program = 
"\
qreg q[5];\
qreg r[5];\
CX q[0], q[1];\
";

        const std::string flattened = 
"\
qreg q[5];\
qreg r[5];\
CX q[0], q[1];\
";
        std::unique_ptr<QModule> qmod = QModule::ParseString(program); 
        FlattenPass* pass = FlattenPass::Create(qmod.get());
        ASSERT_FALSE(pass == nullptr);

        qmod->runPass(pass);
        ASSERT_EQ(qmod->toString(), flattened);
    }

    {
        const std::string program = 
"\
qreg q[5];\
qreg r[5];\
CX q, r[1];\
";

        const std::string flattened = 
"\
qreg q[5];\
qreg r[5];\
CX q[0], r[1];\
CX q[1], r[1];\
CX q[2], r[1];\
CX q[3], r[1];\
CX q[4], r[1];\
";
        std::unique_ptr<QModule> qmod = QModule::ParseString(program); 
        FlattenPass* pass = FlattenPass::Create(qmod.get());
        ASSERT_FALSE(pass == nullptr);

        qmod->runPass(pass);
        ASSERT_EQ(qmod->toString(), flattened);
    }

    {
        const std::string program = 
"\
qreg q[5];\
qreg r[5];\
CX q[0], r;\
";

        const std::string flattened = 
"\
qreg q[5];\
qreg r[5];\
CX q[0], r[0];\
CX q[0], r[1];\
CX q[0], r[2];\
CX q[0], r[3];\
CX q[0], r[4];\
";
        std::unique_ptr<QModule> qmod = QModule::ParseString(program); 
        FlattenPass* pass = FlattenPass::Create(qmod.get());
        ASSERT_FALSE(pass == nullptr);

        qmod->runPass(pass);
        ASSERT_EQ(qmod->toString(), flattened);
    }

    {
        const std::string program = 
"\
qreg q[5];\
qreg r[5];\
CX q, r;\
";

        const std::string flattened = 
"\
qreg q[5];\
qreg r[5];\
CX q[0], r[0];\
CX q[1], r[1];\
CX q[2], r[2];\
CX q[3], r[3];\
CX q[4], r[4];\
";
        std::unique_ptr<QModule> qmod = QModule::ParseString(program); 
        FlattenPass* pass = FlattenPass::Create(qmod.get());
        ASSERT_FALSE(pass == nullptr);

        qmod->runPass(pass);
        ASSERT_EQ(qmod->toString(), flattened);
    }
}

TEST(FlattenPassTests, QGenericOPTest) {
    {
        const std::string program = 
"\
qreg q0[2];\
qreg q1[2];\
qreg q2[5];\
qreg q3[5];\
gate somegate(a, b) w, x, y, z {\
CX w, x;\
CX x, y;\
CX y, z;\
CX z, w;\
}\
somegate(pi, 2) q0[0], q1[1], q2[2], q3[3];\
";

        const std::string flattened = 
"\
qreg q0[2];\
qreg q1[2];\
qreg q2[5];\
qreg q3[5];\
gate somegate(a, b) w, x, y, z {\
CX w, x;\
CX x, y;\
CX y, z;\
CX z, w;\
}\
somegate(pi, 2) q0[0], q1[1], q2[2], q3[3];\
";
        std::unique_ptr<QModule> qmod = QModule::ParseString(program); 
        FlattenPass* pass = FlattenPass::Create(qmod.get());
        ASSERT_FALSE(pass == nullptr);

        qmod->runPass(pass);
        ASSERT_EQ(qmod->toString(), flattened);
    }

    {
        const std::string program = 
"\
qreg q0[2];\
qreg q1[2];\
qreg q2[5];\
qreg q3[5];\
gate somegate(a, b) w, x, y, z {\
CX w, x;\
CX x, y;\
CX y, z;\
CX z, w;\
}\
somegate(pi, 2) q0[0], q1[1], q2[2], q3;\
";

        const std::string flattened = 
"\
qreg q0[2];\
qreg q1[2];\
qreg q2[5];\
qreg q3[5];\
gate somegate(a, b) w, x, y, z {\
CX w, x;\
CX x, y;\
CX y, z;\
CX z, w;\
}\
somegate(pi, 2) q0[0], q1[1], q2[2], q3[0];\
somegate(pi, 2) q0[0], q1[1], q2[2], q3[1];\
somegate(pi, 2) q0[0], q1[1], q2[2], q3[2];\
somegate(pi, 2) q0[0], q1[1], q2[2], q3[3];\
somegate(pi, 2) q0[0], q1[1], q2[2], q3[4];\
";
        std::unique_ptr<QModule> qmod = QModule::ParseString(program); 
        FlattenPass* pass = FlattenPass::Create(qmod.get());
        ASSERT_FALSE(pass == nullptr);

        qmod->runPass(pass);
        ASSERT_EQ(qmod->toString(), flattened);
    }

    {
        const std::string program = 
"\
qreg q0[2];\
qreg q1[2];\
qreg q2[5];\
qreg q3[5];\
gate somegate(a, b) w, x, y, z {\
CX w, x;\
CX x, y;\
CX y, z;\
CX z, w;\
}\
somegate(pi, 2) q0[0], q2, q1[0], q3;\
";

        const std::string flattened = 
"\
qreg q0[2];\
qreg q1[2];\
qreg q2[5];\
qreg q3[5];\
gate somegate(a, b) w, x, y, z {\
CX w, x;\
CX x, y;\
CX y, z;\
CX z, w;\
}\
somegate(pi, 2) q0[0], q2[0], q1[0], q3[0];\
somegate(pi, 2) q0[0], q2[1], q1[0], q3[1];\
somegate(pi, 2) q0[0], q2[2], q1[0], q3[2];\
somegate(pi, 2) q0[0], q2[3], q1[0], q3[3];\
somegate(pi, 2) q0[0], q2[4], q1[0], q3[4];\
";
        std::unique_ptr<QModule> qmod = QModule::ParseString(program); 
        FlattenPass* pass = FlattenPass::Create(qmod.get());
        ASSERT_FALSE(pass == nullptr);

        qmod->runPass(pass);
        ASSERT_EQ(qmod->toString(), flattened);
    }

}

TEST(FlattenPassTests, IfStmtTest) {
    {
        const std::string program = 
"\
qreg q0[2];\
qreg q1[2];\
qreg q2[5];\
qreg q3[5];\
creg c[5];\
gate somegate(a, b) w, x, y, z {\
CX w, x;\
CX x, y;\
CX y, z;\
CX z, w;\
}\
if (c == 5) somegate q0[0], q1[0], q2[0], q3[0];\
";

        const std::string flattened = 
"\
qreg q0[2];\
qreg q1[2];\
qreg q2[5];\
qreg q3[5];\
creg c[5];\
gate somegate(a, b) w, x, y, z {\
CX w, x;\
CX x, y;\
CX y, z;\
CX z, w;\
}\
if (c == 5) somegate q0[0], q1[0], q2[0], q3[0];\
";
        std::unique_ptr<QModule> qmod = QModule::ParseString(program); 
        FlattenPass* pass = FlattenPass::Create(qmod.get());
        ASSERT_FALSE(pass == nullptr);

        qmod->runPass(pass);
        ASSERT_EQ(qmod->toString(), flattened);
    }

    {
        const std::string program = 
"\
qreg q0[2];\
qreg q1[2];\
qreg q2[5];\
qreg q3[5];\
creg c[5];\
gate somegate(a, b) w, x, y, z {\
CX w, x;\
CX x, y;\
CX y, z;\
CX z, w;\
}\
if (c == 5) somegate q0[0], q1[0], q2[0], q3;\
";

        const std::string flattened = 
"\
qreg q0[2];\
qreg q1[2];\
qreg q2[5];\
qreg q3[5];\
creg c[5];\
gate somegate(a, b) w, x, y, z {\
CX w, x;\
CX x, y;\
CX y, z;\
CX z, w;\
}\
if (c == 5) somegate q0[0], q1[0], q2[0], q3[0];\
if (c == 5) somegate q0[0], q1[0], q2[0], q3[1];\
if (c == 5) somegate q0[0], q1[0], q2[0], q3[2];\
if (c == 5) somegate q0[0], q1[0], q2[0], q3[3];\
if (c == 5) somegate q0[0], q1[0], q2[0], q3[4];\
";
        std::unique_ptr<QModule> qmod = QModule::ParseString(program); 
        FlattenPass* pass = FlattenPass::Create(qmod.get());
        ASSERT_FALSE(pass == nullptr);

        qmod->runPass(pass);
        ASSERT_EQ(qmod->toString(), flattened);
    }

    {
        const std::string program = 
"\
qreg q0[2];\
qreg q1[2];\
qreg q2[5];\
qreg q3[5];\
creg c[5];\
gate somegate(a, b) w, x, y, z {\
CX w, x;\
CX x, y;\
CX y, z;\
CX z, w;\
}\
if (c == 5) somegate q0[0], q1[0], q2, q3;\
";

        const std::string flattened = 
"\
qreg q0[2];\
qreg q1[2];\
qreg q2[5];\
qreg q3[5];\
creg c[5];\
gate somegate(a, b) w, x, y, z {\
CX w, x;\
CX x, y;\
CX y, z;\
CX z, w;\
}\
if (c == 5) somegate q0[0], q1[0], q2[0], q3[0];\
if (c == 5) somegate q0[0], q1[0], q2[1], q3[1];\
if (c == 5) somegate q0[0], q1[0], q2[2], q3[2];\
if (c == 5) somegate q0[0], q1[0], q2[3], q3[3];\
if (c == 5) somegate q0[0], q1[0], q2[4], q3[4];\
";
        std::unique_ptr<QModule> qmod = QModule::ParseString(program); 
        FlattenPass* pass = FlattenPass::Create(qmod.get());
        ASSERT_FALSE(pass == nullptr);

        qmod->runPass(pass);
        ASSERT_EQ(qmod->toString(), flattened);
    }
}
