
#include "gtest/gtest.h"

#include "enfield/Analysis/Driver.h"
#include "enfield/Transform/QModule.h"
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
        Node::uRef root = efd::ParseFile(file, dir);
        ASSERT_FALSE(root == nullptr);

        QModule::uRef qmod = efd::QModule::GetFromAST(std::move(root));
        ASSERT_FALSE(qmod.get() == nullptr);
    }
}
