#include "gtest/gtest.h"

#include "enfield/Analysis/QModule.h"
#include "enfield/Analysis/DependencyBuilderPass.h"

#include <string>

using namespace efd;

TEST(QbitToNumberPassTests, WholeProgramTest) {
    {
        const std::string program = \
"\
qreg q[5];\
";

        std::unique_ptr<QModule> qmod = QModule::ParseString(program);
        QbitToNumberPass* pass = QbitToNumberPass::create();
        qmod->runPass(pass);

        ASSERT_TRUE(pass->getUId("q[0]"), 0);
    }


}
