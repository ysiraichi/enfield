#include "gtest/gtest.h"
#include "enfield/Transform/LayersBuilderPass.h"
#include "enfield/Transform/PassCache.h"
#include "enfield/Support/uRefCast.h"

using namespace efd;

static void TestLayersEquality(std::string program,
                               std::vector<std::set<std::string>> rLayers) {
    auto qmod = QModule::ParseString(program);
    auto lbPass = LayersBuilderPass::Create();
    lbPass->run(qmod.get());
    auto layers = lbPass->getData();

    ASSERT_EQ(layers.size(), rLayers.size());

    for (uint32_t i = 0, e = layers.size(); i < e; ++i) {
        auto layer = layers[i];
        auto rLayer = rLayers[i];

        std::set<std::string> layerStringSet;
        for (auto node : layer) layerStringSet.insert(node->toString(false));

        std::vector<std::string> layerStringV(layerStringSet.begin(), layerStringSet.end());
        std::vector<std::string> rLayerStringV(rLayer.begin(), rLayer.end());

        EXPECT_EQ(layerStringV, rLayerStringV);
    }
}

TEST(LayersBuilderPassTests, IdentitySimpleProgram) {
    {
        const std::string program =
"\
OPENQASM 2.0;\
include \"qelib1.inc\";\
qreg q[5];\
cx q[0], q[1];\
";

        TestLayersEquality(program, {{ "cx q[0], q[1];" }});
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

        TestLayersEquality(program, {{ "measure q[0] -> c[1];" }});
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

        TestLayersEquality(program, {
                    { "measure q[0] -> c[1];" },
                    { "if (c == 5) h q[0];"   }
                });
    }
}

TEST(LayersBuilderPassTests, LinearBiggerPrograms) {
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

        TestLayersEquality(program, {
                    { "cx q[0], q[1];" },
                    { "cx q[1], q[2];" },
                    { "cx q[2], q[3];" },
                    { "cx q[3], q[4];" },
                    { "cx q[4], q[0];" }
                });
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

        TestLayersEquality(program, {
                    { "h q[0];" },
                    { "measure q[0] -> c[1];" },
                    { "if (c == 2) h q[1];" },
                    { "h q[1];" },
                    { "measure q[1] -> c[2];" },
                    { "if (c == 2) h q[2];" }
                });
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

        TestLayersEquality(program, {
                    { "h q[0];" },
                    { "measure q[0] -> c[1];" },
                    { "if (c == 2) h q[1];" },
                    { "h q[1];" },
                    { "measure q[1] -> c[2];" },
                    { "if (c == 2) h q[2];" },
                    { "h q[2];" },
                    { "measure q[2] -> c[3];" },
                    { "if (c == 2) h q[3];" },
                    { "h q[3];" },
                    { "measure q[3] -> c[4];" },
                    { "if (c == 2) h q[4];" },
                    { "h q[4];" },
                    { "measure q[4] -> c[0];" }
                });
    }
}

TEST(LayersBuilderPassTests, ManyLayersProgram) {
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

        TestLayersEquality(program, {
                    { "cx q[0], q[1];", "cx q[2], q[3];" },
                    { "cx q[0], q[1];" },
                    { "cx q[0], q[1];" }
                });
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

        TestLayersEquality(program, {
                    { "h q[0];", "h q[1];", "h q[2];", "h q[3];" },
                    {
                        "measure q[0] -> c[0];",
                        "measure q[1] -> c[1];",
                        "measure q[2] -> c[2];",
                        "measure q[3] -> c[3];" 
                    }
                });
    }
}
