#include "gtest/gtest.h"

#include "enfield/Transform/Pass.h"
#include "enfield/Analysis/Nodes.h"
#include "enfield/Analysis/NodeVisitor.h"
#include "enfield/Transform/QModule.h"
#include "enfield/Support/RTTI.h"

#include <string>

using namespace efd;

namespace {
    class ASTVectorVisitor : public NodeVisitor, public PassT<void> {
        public:
            std::vector<Node::Ref> mV;

            ASTVectorVisitor() {}

            void visitNode(Node::Ref ref) {
                mV.push_back(ref);
                visitChildren(ref);
            }

            void visit(NDQasmVersion::Ref ref) override { visitNode(ref); }
            void visit(NDInclude::Ref ref) override { visitNode(ref); }
            void visit(NDRegDecl::Ref ref) override { visitNode(ref); }
            void visit(NDGateDecl::Ref ref) override { visitNode(ref); }
            void visit(NDOpaque::Ref ref) override { visitNode(ref); }
            void visit(NDQOpMeasure::Ref ref) override { visitNode(ref); }
            void visit(NDQOpReset::Ref ref) override { visitNode(ref); }
            void visit(NDQOpU::Ref ref) override { visitNode(ref); }
            void visit(NDQOpCX::Ref ref) override { visitNode(ref); }
            void visit(NDQOpBarrier::Ref ref) override { visitNode(ref); }
            void visit(NDQOpGen::Ref ref) override { visitNode(ref); }
            void visit(NDBinOp::Ref ref) override { visitNode(ref); }
            void visit(NDUnaryOp::Ref ref) override { visitNode(ref); }
            void visit(NDIdRef::Ref ref) override { visitNode(ref); }
            void visit(NDList::Ref ref) override { visitNode(ref); }
            void visit(NDStmtList::Ref ref) override { visitNode(ref); }
            void visit(NDGOpList::Ref ref) override { visitNode(ref); }
            void visit(NDIfStmt::Ref ref) override { visitNode(ref); }
            void visit(NDValue<std::string>::Ref ref) override { visitNode(ref); }
            void visit(NDValue<IntVal>::Ref ref) override { visitNode(ref); }
            void visit(NDValue<RealVal>::Ref ref) override { visitNode(ref); }

            bool run(QModule::Ref qmod) override {
                mV.clear();

                auto version = qmod->getVersion();
                if (version != nullptr)
                    qmod->getVersion()->apply(this);

                for (auto it = qmod->reg_begin(), e = qmod->reg_end(); it != e; ++it) {
                    (*it)->apply(this);
                }

                for (auto it = qmod->gates_begin(), e = qmod->gates_end(); it != e; ++it) {
                    (*it)->apply(this);
                }

                for (auto it = qmod->stmt_begin(), e = qmod->stmt_end(); it != e; ++it) {
                    (*it)->apply(this);
                }

                return false;
            }
    };
}

void compareClonedPrograms(const std::string program) {
    std::unique_ptr<QModule> qmod = QModule::ParseString(program);
    std::unique_ptr<QModule> clone = qmod->clone();

    ASTVectorVisitor vQMod, vClone;
    vQMod.run(qmod.get());
    vClone.run(clone.get());

    ASSERT_EQ(qmod->toString(), clone->toString());
    ASSERT_EQ(vQMod.mV.size(), vClone.mV.size());

    for (uint32_t i = 0, e = vQMod.mV.size(); i < e; ++i)
        ASSERT_FALSE(vQMod.mV[i] == vClone.mV[i]);
}

#define TEST_CLONE(TestName, Program) \
    TEST(NodeCloneTests, TestName) { \
        compareClonedPrograms(Program); \
    }

TEST_CLONE(QASMVersionTest, "OPENQASM 2.0;");
TEST_CLONE(IncludeTest,"include \"qelib1.inc\";");
TEST_CLONE(DeclTest, "qreg q0[10];qreg q1[10];creg c0[10];");
TEST_CLONE(GateTest, "gate notid a {}");
TEST_CLONE(OpaqueGateTest, "opaque ogate(x, y) a, b, c;");
TEST_CLONE(MeasureTest, "measure q[0] -> c[0];");
TEST_CLONE(ResetTest, "reset q0[0];");
TEST_CLONE(BarrierTest, "barrier q0, q1;");
TEST_CLONE(GenericTest, "gate notid(cc) a, b { CX a, b; } notid(pi + 3 / 8) q0[0], q[1];");
TEST_CLONE(CXTest, "CX q0[0], q[1];");
TEST_CLONE(UTest, "U q0[0];");
TEST_CLONE(GOPListTest, "gate notid a, b { CX a, b; U(pi) a; }");

TEST_CLONE(WholeProgramTest, 
"\
OPENQASM 2.0;\
include \"qelib1.inc\";\
qreg q0[10];\
qreg q1[10];\
creg c0[10];\
gate notid(cc) a, b {\
    CX a, b;\
    U(cc) a;\
}\
opaque ogate(x, y) a, b, c;\
measure q[0] -> c[0];\
reset q0[0];\
barrier q0, q1;\
notid(pi + 3 / 8) q0[0], q[1];\
");
