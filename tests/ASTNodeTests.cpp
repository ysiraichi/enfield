
#include "gtest/gtest.h"

#include "enfield/Analysis/Nodes.h"
#include "enfield/Support/RTTI.h"

#include <string>

using namespace efd;

static NodeRef Id(std::string s);
static NodeRef IdRef(std::string id, std::string sz);
static NodeRef IdGate(std::string id);
static NodeRef Int(std::string s);
static NodeRef Real(std::string s);

static NodeRef Id(std::string s) {
    return NDId::Create(s);
}

static NodeRef IdRef(std::string id, std::string sz) {
    return NDIdRef::Create(Id(id), Int(sz));
}

static NodeRef IdGate(std::string id, std::string par) {
    NodeRef refQAL = NDList::Create();
    NodeRef refAL = NDList::Create();
    dynCast<NDList>(refQAL)->addChild(Id(id));
    dynCast<NDList>(refAL)->addChild(Int(par));
    return NDQOpGeneric::Create(Id("id"), refAL, refQAL);
}

static NodeRef Int(std::string s) {
    return NDInt::Create(s);
}

static NodeRef Real(std::string s) {
    return NDReal::Create(s);
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
    NodeRef refId = NDId::Create(idStr);
    TestPrinting(refId, idStr);

    std::string intStr = "10";
    NodeRef refInt = NDInt::Create(intStr);
    TestPrinting(refInt, intStr);

    std::string dStr = "3.14159";
    NodeRef refDv = NDReal::Create(dStr);
    TestPrinting(refDv, dStr);

    std::string rStr = "r0[3]";
    NodeRef refIdRef = NDIdRef::Create(Id("r0"), Int("3"));
    TestPrinting(refIdRef, rStr);
}

TEST(ASTNodeTests, BinOpCreationTest) {
    std::string addStr = "(pi + 5)";
    NodeRef refAdd = NDBinOp::Create(NDBinOp::OP_ADD, Id("pi"), Int("5"));
    TestPrinting(refAdd, addStr);

    std::string subStr = "(pi - 5)";
    NodeRef refSub = NDBinOp::Create(NDBinOp::OP_SUB, Id("pi"), Int("5"));
    TestPrinting(refSub, subStr);

    std::string mulStr = "(pi * 5)";
    NodeRef refMul = NDBinOp::Create(NDBinOp::OP_MUL, Id("pi"), Int("5"));
    TestPrinting(refMul, mulStr);

    std::string divStr = "(pi / 5)";
    NodeRef refDiv = NDBinOp::Create(NDBinOp::OP_DIV, Id("pi"), Int("5"));
    TestPrinting(refDiv, divStr);

    std::string powStr = "(pi ^ 5)";
    NodeRef refPow = NDBinOp::Create(NDBinOp::OP_POW, Id("pi"), Int("5"));
    TestPrinting(refPow, powStr);

    std::string mixStr = "((pi ^ 5) / (2 * 5.8))";
    NodeRef refLhs = NDBinOp::Create(NDBinOp::OP_POW, Id("pi"), Int("5"));
    NodeRef refRhs = NDBinOp::Create(NDBinOp::OP_MUL, Int("2"), Real("5.8"));
    NodeRef refMix = NDBinOp::Create(NDBinOp::OP_DIV, refLhs, refRhs);
    TestPrinting(refMix, mixStr);
}

TEST(ASTNodeTests, UnaryOpCreationTest) {
    std::string sinStr = "sin(pi)";
    NodeRef refSin = NDUnaryOp::Create(NDUnaryOp::UOP_SIN, Id("pi"));
    TestPrinting(refSin, sinStr);

    std::string cosStr = "cos(pi)";
    NodeRef refCos = NDUnaryOp::Create(NDUnaryOp::UOP_COS, Id("pi"));
    TestPrinting(refCos, cosStr);

    std::string tanStr = "tan(pi)";
    NodeRef refTan = NDUnaryOp::Create(NDUnaryOp::UOP_TAN, Id("pi"));
    TestPrinting(refTan, tanStr);

    std::string lnStr = "ln(pi)";
    NodeRef refLn = NDUnaryOp::Create(NDUnaryOp::UOP_LN, Id("pi"));
    TestPrinting(refLn, lnStr);

    std::string negStr = "(-pi)";
    NodeRef refNeg = NDUnaryOp::Create(NDUnaryOp::UOP_NEG, Id("pi"));
    TestPrinting(refNeg, negStr);

    std::string expStr = "exp(pi)";
    NodeRef refExp = NDUnaryOp::Create(NDUnaryOp::UOP_EXP, Id("pi"));
    TestPrinting(refExp, expStr);

    std::string sqrtStr = "sqrt(pi)";
    NodeRef refSqrt = NDUnaryOp::Create(NDUnaryOp::UOP_SQRT, Id("pi"));
    TestPrinting(refSqrt, sqrtStr);
}

TEST(ASTNodeTests, DeclCreationTest) {
    std::string cStr = "creg r0[5];";
    NodeRef refC = NDDecl::Create(NDDecl::CONCRETE, Id("r0"), Int("5"));
    TestPrinting(refC, cStr);

    std::string qStr = "qreg r0[5];";
    NodeRef refQ = NDDecl::Create(NDDecl::QUANTUM, Id("r0"), Int("5"));
    TestPrinting(refQ, qStr);
}

TEST(ASTNodeTests, ListCreationTest) {
    std::string argStr = "r0, r1, r2[0], r3[4]";

    NodeRef refQArgs = NDList::Create();
    NDList* argList = dynCast<NDList>(refQArgs);

    argList->addChild(Id("r0"));
    argList->addChild(Id("r1"));
    argList->addChild(IdRef("r2", "0"));
    argList->addChild(IdRef("r3", "4"));

    TestPrinting(refQArgs, argStr);
}

TEST(ASTNodeTests, QOpCreationTest) {
    std::string meaStr = "measure r0[2] -> c2[5];";
    NodeRef refMeasure = NDQOpMeasure::Create(IdRef("r0", "2"), IdRef("c2", "5"));
    TestPrinting(refMeasure, meaStr);

    std::string resetStr = "reset r0[5];";
    NodeRef refReset = NDQOpReset::Create(IdRef("r0", "5"));
    TestPrinting(refReset, resetStr);

    std::string barrierStr = "barrier r0[2], r1[5], r3, r5;";
    NodeRef refBarrier;
    {
        NodeRef refArgList = NDList::Create();
        NDList *list = dynCast<NDList>(refArgList);
        list->addChild(IdRef("r0", "2"));
        list->addChild(IdRef("r1", "5"));
        list->addChild(Id("r3"));
        list->addChild(Id("r5"));
        refBarrier = NDQOpBarrier::Create(refArgList);
    }
    TestPrinting(refBarrier, barrierStr);

    std::string genStr = "id r0, r5;";
    NodeRef refGeneric;
    {
        NodeRef refArgList = NDList::Create();
        NDList *list = dynCast<NDList>(refArgList);
        list->addChild(Id("r0"));
        list->addChild(Id("r5"));
        refGeneric = NDQOpGeneric::Create(Id("id"), NDList::Create(), refArgList);
    }
    TestPrinting(refGeneric, genStr);
}

TEST(ASTNodeTests, GOpListCreationTest) {
    std::string gopStr = "id(3) r0;id(5) r5;id(7) r3;";

    NodeRef refGOp = NDGOpList::Create();
    NDGOpList* gopList = dynCast<NDGOpList>(refGOp);

    gopList->addChild(IdGate("r0", "3"));
    gopList->addChild(IdGate("r5", "5"));
    gopList->addChild(IdGate("r3", "7"));

    TestPrinting(refGOp, gopStr);
}

TEST(ASTNodeTests, StmtListCreationTest) {
    std::string stmtStr = "id(3) r0;id(5) r5;id(7) r3;";

    NodeRef refStmt = NDStmtList::Create();
    NDStmtList* stmtList = dynCast<NDStmtList>(refStmt);

    stmtList->addChild(IdGate("r0", "3"));
    stmtList->addChild(IdGate("r5", "5"));
    stmtList->addChild(IdGate("r3", "7"));

    TestPrinting(refStmt, stmtStr);
}

TEST(ASTNodeTests, OpaqueGateDeclCreationTest) {
    std::string opaqueStr = "opaque Ogate(10, sin(pi), (-3.14159)) r0, r5;";

    NodeRef refQArgs = NDList::Create();
    NDList* qaList = dynCast<NDList>(refQArgs);
    qaList->addChild(Id("r0"));
    qaList->addChild(Id("r5"));

    NodeRef refArgs = NDList::Create();
    NDList* aList = dynCast<NDList>(refArgs);
    aList->addChild(Int("10"));
    aList->addChild(NDUnaryOp::Create(NDUnaryOp::UOP_SIN, Id("pi")));
    aList->addChild(NDUnaryOp::Create(NDUnaryOp::UOP_NEG, Real("3.14159")));

    NodeRef refOpaque = NDOpaque::Create(Id("Ogate"), refArgs, refQArgs);
    TestPrinting(refOpaque, opaqueStr);
}

TEST(ASTNodeTests, GateDeclCreationTest) {
    std::string idGateStr = "gate id r0 {}";

    NodeRef refQArgs = NDList::Create();
    dynCast<NDList>(refQArgs)->addChild(Id("r0"));

    NodeRef refIdGate = NDGateDecl::Create(Id("id"), NDList::Create(), refQArgs, 
            NDGOpList::Create());
    TestPrinting(refIdGate, idGateStr);
}

TEST(ASTNodeTests, IfStmtTest) {
    std::string ifStr = "if (someid == 1) reset otherid[0];";

    NodeRef refReset = NDQOpReset::Create(NDIdRef::Create(Id("otherid"), Int("0")));
    NodeRef refIf = NDIfStmt::Create(Id("someid"), Int("1"), refReset);
    TestPrinting(refIf, ifStr);
}

TEST(ASTNodeTests, IncludeTest) {
    std::string includeStr = "include \"files/_qelib1.inc\";";

    NodeRef refInclude = NDInclude::Create(Id("files/_qelib1.inc"), NDStmtList::Create());
    TestPrinting(refInclude, includeStr);
}
