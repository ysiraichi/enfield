
#include "gtest/gtest.h"

#include "enfield/Analysis/Driver.h"
#include "enfield/Analysis/QModule.h"
#include "enfield/Support/RTTI.h"

#include <string>

using namespace efd;

const std::string dir = "files/";
const std::vector<std::string> files = {
    "adder.qasm",
    "bigadder.qasm",
    "inverseqft1.qasm",
    "inverseqft2.qasm",
    "qec.qasm",
    "qft.qasm",
    "qpt.qasm",
    "rb.qasm",
    "teleport.qasm",
    "W-state.qasm"
};

TEST(QModuleTests, TransformationTest) {
    for (const std::string& file : files) {
        NodeRef root = efd::ParseFile(file, dir);
        ASSERT_FALSE(root == nullptr);

        std::unique_ptr<QModule> qmod = efd::QModule::GetFromAST(root);
        ASSERT_FALSE(qmod.get() == nullptr);
    }
}
