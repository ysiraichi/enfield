#include "gtest/gtest.h"

#include "enfield/Analysis/Driver.h"
#include "enfield/Support/RTTI.h"

#include <vector>
#include <fstream>
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

TEST(DriverFileTests, ParsingFilesTest) {
    for (const std::string& file : files) {
        auto root = efd::ParseFile(file, dir);
        ASSERT_FALSE(root.get() == nullptr);
    }
}
