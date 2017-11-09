#include "gtest/gtest.h"
#include "enfield/Transform/CNOTLBOWrapperPass.h"
#include "enfield/Transform/PassCache.h"
#include "enfield/Support/uRefCast.h"

using namespace efd;

static void CheckOrdering(std::string program, std::vector<uint32_t> order) {
    auto qmod = QModule::ParseString(program);
    auto cnotLBOpass = CNOTLBOWrapperPass::Create();
    cnotLBOpass->run(qmod.get());
    auto passdata = cnotLBOpass->getData();
    ASSERT_EQ(passdata.ordering, order);

    PassCache::Clear(qmod.get());
}

TEST(CNOTLBOWrapperPassTests, IdentitySimpleProgram) {
    {
        const std::string program =
"\
OPENQASM 2.0;\
include \"qelib1.inc\";\
qreg q[5];\
cx q[0], q[1];\
";

        CheckOrdering(program, { 0 });
    }
    {
        const std::string program =
"\
OPENQASM 2.0;\
include \"qelib1.inc\";\
qreg q[5];\
creg c[5];\
measure q[0] -> c[1];\
";

        CheckOrdering(program, { 0 });
    }
    {
        const std::string program =
"\
OPENQASM 2.0;\
include \"qelib1.inc\";\
qreg q[5];\
creg c[5];\
measure q[0] -> c[1];\
if (c == 5) h q[0];\
";

        CheckOrdering(program, { 0, 1 });
    }
}

TEST(CNOTLBOWrapperPassTests, IdentityComplexProgram) {
    {
        const std::string program =
"\
OPENQASM 2.0;\
include \"qelib1.inc\";\
qreg q[5];\
cx q[0], q[1];\
cx q[1], q[2];\
cx q[2], q[3];\
cx q[3], q[4];\
cx q[4], q[0];\
";

        CheckOrdering(program, { 0, 1, 2, 3, 4 });
    }
    {
        const std::string program =
"\
OPENQASM 2.0;\
include \"qelib1.inc\";\
qreg q[5];\
creg c[5];\
h q[0];\
measure q[0] -> c[1];\
if (c == 2) h q[1];\
h q[1];\
measure q[1] -> c[2];\
if (c == 2) h q[2];\
";

        CheckOrdering(program, { 0, 1, 2, 3, 4, 5 });
    }
    {
        const std::string program =
"\
OPENQASM 2.0;\
include \"qelib1.inc\";\
qreg q[5];\
creg c[5];\
h q[0];\
measure q[0] -> c[1];\
if (c == 2) h q[1];\
h q[1];\
measure q[1] -> c[2];\
if (c == 2) h q[2];\
h q[2];\
measure q[2] -> c[3];\
if (c == 2) h q[3];\
h q[3];\
measure q[3] -> c[4];\
if (c == 2) h q[4];\
h q[4];\
measure q[4] -> c[0];\
";

        CheckOrdering(program, { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13 });
    }
}

TEST(CNOTLBOWrapperPassTests, ReorderSimpleProgram) {
    {
        const std::string program =
"\
OPENQASM 2.0;\
include \"qelib1.inc\";\
qreg q[5];\
cx q[0], q[1];\
cx q[0], q[1];\
cx q[2], q[3];\
cx q[0], q[1];\
";

        CheckOrdering(program, { 0, 2, 1, 3 });
    }
    {
        const std::string program =
"\
OPENQASM 2.0;\
include \"qelib1.inc\";\
qreg q[5];\
creg c[5];\
h q[0];\
measure q[0] -> c[0];\
h q[1];\
measure q[1] -> c[1];\
h q[2];\
measure q[2] -> c[2];\
h q[3];\
measure q[3] -> c[3];\
";

        CheckOrdering(program, { 0, 2, 4, 6, 1, 3, 5, 7 });
    }
}
