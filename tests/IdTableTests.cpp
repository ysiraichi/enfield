#include "gtest/gtest.h"

#include "enfield/Transform/IdTable.h"
#include "enfield/Transform/QModule.h"
#include "enfield/Support/RTTI.h"

#include <string>

using namespace efd;

const std::string program = \
"\
OPENQASM 2.0;\
include \"files/qelib1.inc\";\
gate id a {\
}\
gate cnot a, b {\
    cx a, b;\
}\
qreg q[5];\
creg c[5];\
id q[0];\
id q[1];\
cnot q[0], q;\
measure q[0] -> c[0];\
measure q[1] -> c[1];\
measure q[2] -> c[2];\
measure q[3] -> c[3];\
measure q[4] -> c[4];\
";

TEST(IdTableTests, SingletonTest) {
    std::unique_ptr<QModule> mod = QModule::ParseString(program);

    ASSERT_FALSE(mod->getQVar("q") == nullptr);
    ASSERT_FALSE(mod->getQVar("c") == nullptr);
    ASSERT_TRUE(mod->getQVar("id") == nullptr);
    ASSERT_TRUE(mod->getQVar("cnot") == nullptr);

    ASSERT_TRUE(mod->getQGate("q") == nullptr);
    ASSERT_TRUE(mod->getQGate("c") == nullptr);
    ASSERT_FALSE(mod->getQGate("id") == nullptr);
    ASSERT_FALSE(mod->getQGate("cnot") == nullptr);

    NDGateDecl* ref;
    ref = mod->getQGate("id");
    ASSERT_FALSE(ref == nullptr);
    ASSERT_FALSE(mod->getIdTable(ref).getQVar("a", false) == nullptr);
    ASSERT_FALSE(mod->getIdTable(ref).getQVar("q", true) == nullptr);
    ASSERT_FALSE(mod->getIdTable(ref).getQVar("c", true) == nullptr);
    ASSERT_TRUE(mod->getIdTable(ref).getQVar("q", false) == nullptr);
    ASSERT_TRUE(mod->getIdTable(ref).getQVar("c", false) == nullptr);

    ref = mod->getQGate("cnot");
    ASSERT_FALSE(ref == nullptr);
    ASSERT_FALSE(mod->getIdTable(ref).getQVar("a", false) == nullptr);
    ASSERT_FALSE(mod->getIdTable(ref).getQVar("b", false) == nullptr);
    ASSERT_FALSE(mod->getIdTable(ref).getQVar("q", true) == nullptr);
    ASSERT_FALSE(mod->getIdTable(ref).getQVar("c", true) == nullptr);
    ASSERT_TRUE(mod->getIdTable(ref).getQVar("q", false) == nullptr);
    ASSERT_TRUE(mod->getIdTable(ref).getQVar("c", false) == nullptr);
}
