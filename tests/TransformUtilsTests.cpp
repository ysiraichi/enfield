
#include "gtest/gtest.h"

#include "enfield/Transform/Utils.h"
#include "enfield/Transform/QModule.h"
#include "enfield/Support/RTTI.h"

#include <string>

using namespace efd;

TEST(GateInlineTests, IdInline) {
    const std::string program =
"\
gate id a {\
}\
qreg q[5];\
id q[0];\
";

    const std::string inlined =
"\
include \"qelib1.inc\";\
qreg q[5];\
";

    std::unique_ptr<QModule> qmod = QModule::ParseString(program);
    for (auto it = qmod->stmt_begin(), e = qmod->stmt_end(); it != e; ++it)
        if (NDQOp* qop = dynCast<NDQOp>(it->get()))
            InlineGate(qmod.get(), qop);

    ASSERT_EQ(qmod->toString(), inlined);
}

TEST(GateInlineTests, PrimitiveGatesInline) {
    const std::string program =
"\
include \"qelib1.inc\";\
gate my_ccx a, b, c {\
h c;\
cx b, c;\
tdg c;\
cx a, c;\
t c;\
cx b, c;\
tdg c;\
cx a, c;\
t b;\
t c;\
h c;\
cx a, b;\
t a;\
tdg b;\
cx a, b;\
}\
qreg q[5];\
my_ccx q[0], q[1], q[2];\
";

    const std::string inlined =
"\
include \"qelib1.inc\";\
qreg q[5];\
h q[2];\
cx q[1], q[2];\
tdg q[2];\
cx q[0], q[2];\
t q[2];\
cx q[1], q[2];\
tdg q[2];\
cx q[0], q[2];\
t q[1];\
t q[2];\
h q[2];\
cx q[0], q[1];\
t q[0];\
tdg q[1];\
cx q[0], q[1];\
";

    std::unique_ptr<QModule> qmod = QModule::ParseString(program);
    for (auto it = qmod->stmt_begin(), e = qmod->stmt_end(); it != e; ++it)
        if (NDQOp* qop = dynCast<NDQOp>(it->get()))
            InlineGate(qmod.get(), qop);

    ASSERT_EQ(qmod->toString(), inlined);
}
