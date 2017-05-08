
#include "gtest/gtest.h"

#include "enfield/Analysis/Nodes.h"
#include "enfield/Support/RTTI.h"

#include <string>

using namespace efd;
typedef Node::NodeRef NodeRef;

static NodeRef Id(std::string s);
static NodeRef IdRef(std::string id, std::string sz);
static NodeRef IdGate(std::string id);
static NodeRef Int(std::string s);
static NodeRef Real(std::string s);

static NodeRef Id(std::string s) {
    return NDId::create(s);
}

static NodeRef IdRef(std::string id, std::string sz) {
    return NDIdRef::create(std::move(Id(id)), std::move(Int(sz)));
}

static NodeRef IdGate(std::string id, std::string par) {
    NodeRef refQAL = NDArgList::create();
    NodeRef refAL = NDArgList::create();
    dynCast<NDArgList>(refQAL.get())->addChild(std::move(Id(id)));
    dynCast<NDArgList>(refAL.get())->addChild(std::move(Int(par)));
    return NDQOpGeneric::create(std::move(Id("id")), std::move(refAL), std::move(refQAL));
}

static NodeRef Int(std::string s) {
    return NDInt::create(std::stoi(s));
}

static NodeRef Real(std::string s) {
    return NDReal::create(s);
}

static void TestPrinting(Node* node, std::string rhs) {
    ASSERT_FALSE(node == nullptr);
    ASSERT_EQ(node->toString(), rhs);

    std::ostringstream ss;
    node->print(ss);
    ASSERT_EQ(ss.str(), rhs);
}

TEST(ASTNodeTests, LiteralCreationTest) {
    std::string idStr = "r0";
    NodeRef refId = NDId::create(idStr);
    TestPrinting(refId.get(), idStr);

    std::string intStr = "10";
    NodeRef refInt = NDInt::create(std::stoi(intStr));
    TestPrinting(refInt.get(), intStr);

    std::string dStr = "3.14159";
    NodeRef refDv = NDReal::create(dStr);
    TestPrinting(refDv.get(), dStr);

    std::string rStr = "r0[3]";
    NodeRef refIdRef = NDIdRef::create(std::move(Id("r0")), std::move(Int("3")));
    TestPrinting(refIdRef.get(), rStr);
}

TEST(ASTNodeTests, BinOpCreationTest) {
    std::string addStr = "(pi + 5)";
    NodeRef refAdd = NDBinOp::create(NDBinOp::OP_ADD, std::move(Id("pi")), std::move(Int("5")));
    TestPrinting(refAdd.get(), addStr);

    std::string subStr = "(pi - 5)";
    NodeRef refSub = NDBinOp::create(NDBinOp::OP_SUB, std::move(Id("pi")), std::move(Int("5")));
    TestPrinting(refSub.get(), subStr);

    std::string mulStr = "(pi * 5)";
    NodeRef refMul = NDBinOp::create(NDBinOp::OP_MUL, std::move(Id("pi")), std::move(Int("5")));
    TestPrinting(refMul.get(), mulStr);

    std::string divStr = "(pi / 5)";
    NodeRef refDiv = NDBinOp::create(NDBinOp::OP_DIV, std::move(Id("pi")), std::move(Int("5")));
    TestPrinting(refDiv.get(), divStr);

    std::string powStr = "(pi ^ 5)";
    NodeRef refPow = NDBinOp::create(NDBinOp::OP_POW, std::move(Id("pi")), std::move(Int("5")));
    TestPrinting(refPow.get(), powStr);

    std::string mixStr = "((pi ^ 5) / (2 * 5.8))";
    NodeRef refLhs = NDBinOp::create(NDBinOp::OP_POW, std::move(Id("pi")), std::move(Int("5")));
    NodeRef refRhs = NDBinOp::create(NDBinOp::OP_MUL, std::move(Int("2")), std::move(Real("5.8")));
    NodeRef refMix = NDBinOp::create(NDBinOp::OP_DIV, std::move(refLhs), std::move(refRhs));
    TestPrinting(refMix.get(), mixStr);
}

TEST(ASTNodeTests, UnaryOpCreationTest) {
    std::string sinStr = "sin(pi)";
    NodeRef refSin = NDUnaryOp::create(NDUnaryOp::UOP_SIN, std::move(Id("pi")));
    TestPrinting(refSin.get(), sinStr);

    std::string cosStr = "cos(pi)";
    NodeRef refCos = NDUnaryOp::create(NDUnaryOp::UOP_COS, std::move(Id("pi")));
    TestPrinting(refCos.get(), cosStr);

    std::string tanStr = "tan(pi)";
    NodeRef refTan = NDUnaryOp::create(NDUnaryOp::UOP_TAN, std::move(Id("pi")));
    TestPrinting(refTan.get(), tanStr);

    std::string lnStr = "ln(pi)";
    NodeRef refLn = NDUnaryOp::create(NDUnaryOp::UOP_LN, std::move(Id("pi")));
    TestPrinting(refLn.get(), lnStr);

    std::string negStr = "(-pi)";
    NodeRef refNeg = NDUnaryOp::create(NDUnaryOp::UOP_NEG, std::move(Id("pi")));
    TestPrinting(refNeg.get(), negStr);

    std::string expStr = "exp(pi)";
    NodeRef refExp = NDUnaryOp::create(NDUnaryOp::UOP_EXP, std::move(Id("pi")));
    TestPrinting(refExp.get(), expStr);

    std::string sqrtStr = "sqrt(pi)";
    NodeRef refSqrt = NDUnaryOp::create(NDUnaryOp::UOP_SQRT, std::move(Id("pi")));
    TestPrinting(refSqrt.get(), sqrtStr);
}

TEST(ASTNodeTests, DeclCreationTest) {
    std::string cStr = "creg r0[5];";
    NodeRef refC = NDDecl::create(NDDecl::CONCRETE, std::move(Id("r0")), std::move(Int("5")));
    TestPrinting(refC.get(), cStr);

    std::string qStr = "qreg r0[5];";
    NodeRef refQ = NDDecl::create(NDDecl::QUANTUM, std::move(Id("r0")), std::move(Int("5")));
    TestPrinting(refQ.get(), qStr);
}

TEST(ASTNodeTests, ArgListCreationTest) {
    std::string argStr = "r0, r1, r2[0], r3[4]";

    NodeRef refQArgs = NDArgList::create();
    NDArgList* argList = dynCast<NDArgList>(refQArgs.get());

    argList->addChild(std::move(Id("r0")));
    argList->addChild(std::move(Id("r1")));
    argList->addChild(std::move(IdRef("r2", "0")));
    argList->addChild(std::move(IdRef("r3", "4")));

    TestPrinting(refQArgs.get(), argStr);
}

TEST(ASTNodeTests, QOpCreationTest) {
    std::string meaStr = "measure r0[2] -> c2[5];";
    NodeRef refMeasure = NDQOpMeasure::create(std::move(IdRef("r0", "2")), std::move(IdRef("c2", "5")));
    TestPrinting(refMeasure.get(), meaStr);

    std::string resetStr = "reset r0[5];";
    NodeRef refReset = NDQOpReset::create(std::move(IdRef("r0", "5")));
    TestPrinting(refReset.get(), resetStr);

    std::string barrierStr = "barrier r0[2], r1[5], r3, r5;";
    NodeRef refBarrier;
    {
        NodeRef refArgList = NDArgList::create();
        NDArgList *list = dynCast<NDArgList>(refArgList.get());
        list->addChild(std::move(IdRef("r0", "2")));
        list->addChild(std::move(IdRef("r1", "5")));
        list->addChild(std::move(Id("r3")));
        list->addChild(std::move(Id("r5")));
        refBarrier = NDQOpBarrier::create(std::move(refArgList));
    }
    TestPrinting(refBarrier.get(), barrierStr);

    std::string genStr = "id r0, r5;";
    NodeRef refGeneric;
    {
        NodeRef refArgList = NDArgList::create();
        NDArgList *list = dynCast<NDArgList>(refArgList.get());
        list->addChild(std::move(Id("r0")));
        list->addChild(std::move(Id("r5")));
        refGeneric = NDQOpGeneric::create(std::move(Id("id")), std::move(NDArgList::create()), std::move(refArgList));
    }
    TestPrinting(refGeneric.get(), genStr);
}

TEST(ASTNodeTests, GOpListCreationTest) {
    std::string gopStr = "id(3) r0;id(5) r5;id(7) r3;";

    NodeRef refGOp = NDGOpList::create();
    NDGOpList* gopList = dynCast<NDGOpList>(refGOp.get());

    gopList->addChild(std::move(IdGate("r0", "3")));
    gopList->addChild(std::move(IdGate("r5", "5")));
    gopList->addChild(std::move(IdGate("r3", "7")));

    TestPrinting(refGOp.get(), gopStr);
}

TEST(ASTNodeTests, OpaqueGateDeclCreationTest) {
    std::string opaqueStr = "opaque Ogate(10, sin(pi), (-3.14159)) r0, r5;";

    NodeRef refQArgs = NDArgList::create();
    NDArgList* qaList = dynCast<NDArgList>(refQArgs.get());
    qaList->addChild(std::move(Id("r0")));
    qaList->addChild(std::move(Id("r5")));

    NodeRef refArgs = NDArgList::create();
    NDArgList* aList = dynCast<NDArgList>(refArgs.get());
    aList->addChild(std::move(Int("10")));
    aList->addChild(std::move(NDUnaryOp::create(NDUnaryOp::UOP_SIN, std::move(Id("pi")))));
    aList->addChild(std::move(NDUnaryOp::create(NDUnaryOp::UOP_NEG, std::move(Real("3.14159")))));

    NodeRef refOpaque = NDOpaque::create(std::move(Id("Ogate")), std::move(refArgs), std::move(refQArgs));
    TestPrinting(refOpaque.get(), opaqueStr);
}

TEST(ASTNodeTests, GateDeclCreationTest) {
    std::string idGateStr = "gate id r0 {}";

    NodeRef refQArgs = NDArgList::create();
    dynCast<NDArgList>(refQArgs.get())->addChild(std::move(Id("r0")));

    NodeRef refIdGate = NDGateDecl::create(std::move(Id("id")), std::move(NDArgList::create()), std::move(refQArgs), 
            std::move(NDGOpList::create()));
    TestPrinting(refIdGate.get(), idGateStr);
}
