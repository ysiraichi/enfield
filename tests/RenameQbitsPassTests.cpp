
#include "gtest/gtest.h"

#include "enfield/Transform/RenameQbitsPass.h"
#include "enfield/Analysis/QModule.h"
#include "enfield/Support/RTTI.h"

#include <string>

using namespace efd;

TEST(RenameQbitPassTests, PlainTest) {
    {
        const std::string program =
"\
qreg q[5];\
CX q[0], q[1];\
";
        const std::string result =
"\
qreg q[5];\
CX q[1], q[0];\
";
        std::unique_ptr<QModule> qmod = QModule::ParseString(program);
        
        RenameQbitPass::ArchMap aMap {
            { "q[0]", NDIdRef::Create(NDId::Create("q"), NDInt::Create(std::string("1"))) },
            { "q[1]", NDIdRef::Create(NDId::Create("q"), NDInt::Create(std::string("0"))) }
        };

        RenameQbitPass* pass = RenameQbitPass::Create(aMap);
        qmod->runPass(pass);
    }

    {
        const std::string program =
"\
qreg q[5];\
CX q[0], q[1];\
CX q[1], q[2];\
CX q[2], q[3];\
CX q[3], q[4];\
CX q[4], q[0];\
";
        const std::string result =
"\
qreg q[5];\
CX q[4], q[3];\
CX q[3], q[2];\
CX q[2], q[1];\
CX q[1], q[0];\
CX q[0], q[4];\
";
        std::unique_ptr<QModule> qmod = QModule::ParseString(program);
        
        RenameQbitPass::ArchMap aMap {
            { "q[0]", NDIdRef::Create(NDId::Create("q"), NDInt::Create(std::string("4"))) },
            { "q[1]", NDIdRef::Create(NDId::Create("q"), NDInt::Create(std::string("3"))) },
            { "q[2]", NDIdRef::Create(NDId::Create("q"), NDInt::Create(std::string("2"))) },
            { "q[3]", NDIdRef::Create(NDId::Create("q"), NDInt::Create(std::string("1"))) },
            { "q[4]", NDIdRef::Create(NDId::Create("q"), NDInt::Create(std::string("0"))) }
        };

        RenameQbitPass* pass = RenameQbitPass::Create(aMap);
        qmod->runPass(pass);
    }
}
