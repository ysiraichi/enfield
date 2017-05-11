#include "gtest/gtest.h"

#include "enfield/Analysis/Driver.h"
#include "enfield/Support/RTTI.h"

#include <vector>
#include <fstream>
#include <string>

using namespace efd;

const std::string dir = "files/";
const std::vector<std::string> files = {
    "_adder.qasm",
    "_bigadder.qasm",
    "_inverseqft1.qasm",
    "_inverseqft2.qasm",
    "_qec.qasm",
    "_qft.qasm",
    "_qpt.qasm",
    "_rb.qasm",
    "_teleport.qasm",
    "_W-state.qasm"
};

TEST(DriverFileTests, ParsingFilesTest) {
    for (const std::string& file : files) {
        NodeRef root = efd::ParseFile(file, dir);
        ASSERT_FALSE(root == nullptr);
    }
}
