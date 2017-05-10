
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
    return NDId::create(s);
}

static NodeRef IdRef(std::string id, std::string sz) {
    return NDIdRef::create(Id(id), Int(sz));
}

static NodeRef IdGate(std::string id, std::string par) {
    NodeRef refQAL = NDList::create();
    NodeRef refAL = NDList::create();
    dynCast<NDList>(refQAL)->addChild(Id(id));
    dynCast<NDList>(refAL)->addChild(Int(par));
    return NDQOpGeneric::create(Id("id"), refAL, refQAL);
}

static NodeRef Int(std::string s) {
    return NDInt::create(s);
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
    TestPrinting(refId, idStr);

    std::string intStr = "10";
    NodeRef refInt = NDInt::create(intStr);
    TestPrinting(refInt, intStr);

    std::string dStr = "3.14159";
    NodeRef refDv = NDReal::create(dStr);
    TestPrinting(refDv, dStr);

    std::string rStr = "r0[3]";
    NodeRef refIdRef = NDIdRef::create(Id("r0"), Int("3"));
    TestPrinting(refIdRef, rStr);
}

TEST(ASTNodeTests, BinOpCreationTest) {
    std::string addStr = "(pi + 5)";
    NodeRef refAdd = NDBinOp::create(NDBinOp::OP_ADD, Id("pi"), Int("5"));
    TestPrinting(refAdd, addStr);

    std::string subStr = "(pi - 5)";
    NodeRef refSub = NDBinOp::create(NDBinOp::OP_SUB, Id("pi"), Int("5"));
    TestPrinting(refSub, subStr);

    std::string mulStr = "(pi * 5)";
    NodeRef refMul = NDBinOp::create(NDBinOp::OP_MUL, Id("pi"), Int("5"));
    TestPrinting(refMul, mulStr);

    std::string divStr = "(pi / 5)";
    NodeRef refDiv = NDBinOp::create(NDBinOp::OP_DIV, Id("pi"), Int("5"));
    TestPrinting(refDiv, divStr);

    std::string powStr = "(pi ^ 5)";
    NodeRef refPow = NDBinOp::create(NDBinOp::OP_POW, Id("pi"), Int("5"));
    TestPrinting(refPow, powStr);

    std::string mixStr = "((pi ^ 5) / (2 * 5.8))";
    NodeRef refLhs = NDBinOp::create(NDBinOp::OP_POW, Id("pi"), Int("5"));
    NodeRef refRhs = NDBinOp::create(NDBinOp::OP_MUL, Int("2"), Real("5.8"));
    NodeRef refMix = NDBinOp::create(NDBinOp::OP_DIV, refLhs, refRhs);
    TestPrinting(refMix, mixStr);
}

TEST(ASTNodeTests, UnaryOpCreationTest) {
    std::string sinStr = "sin(pi)";
    NodeRef refSin = NDUnaryOp::create(NDUnaryOp::UOP_SIN, Id("pi"));
    TestPrinting(refSin, sinStr);

    std::string cosStr = "cos(pi)";
    NodeRef refCos = NDUnaryOp::create(NDUnaryOp::UOP_COS, Id("pi"));
    TestPrinting(refCos, cosStr);

    std::string tanStr = "tan(pi)";
    NodeRef refTan = NDUnaryOp::create(NDUnaryOp::UOP_TAN, Id("pi"));
    TestPrinting(refTan, tanStr);

    std::string lnStr = "ln(pi)";
    NodeRef refLn = NDUnaryOp::create(NDUnaryOp::UOP_LN, Id("pi"));
    TestPrinting(refLn, lnStr);

    std::string negStr = "(-pi)";
    NodeRef refNeg = NDUnaryOp::create(NDUnaryOp::UOP_NEG, Id("pi"));
    TestPrinting(refNeg, negStr);

    std::string expStr = "exp(pi)";
    NodeRef refExp = NDUnaryOp::create(NDUnaryOp::UOP_EXP, Id("pi"));
    TestPrinting(refExp, expStr);

    std::string sqrtStr = "sqrt(pi)";
    NodeRef refSqrt = NDUnaryOp::create(NDUnaryOp::UOP_SQRT, Id("pi"));
    TestPrinting(refSqrt, sqrtStr);
}

TEST(ASTNodeTests, DeclCreationTest) {
    std::string cStr = "creg r0[5];";
    NodeRef refC = NDDecl::create(NDDecl::CONCRETE, Id("r0"), Int("5"));
    TestPrinting(refC, cStr);

    std::string qStr = "qreg r0[5];";
    NodeRef refQ = NDDecl::create(NDDecl::QUANTUM, Id("r0"), Int("5"));
    TestPrinting(refQ, qStr);
}

TEST(ASTNodeTests, ListCreationTest) {
    std::string argStr = "r0, r1, r2[0], r3[4]";

    NodeRef refQArgs = NDList::create();
    NDList* argList = dynCast<NDList>(refQArgs);

    argList->addChild(Id("r0"));
    argList->addChild(Id("r1"));
    argList->addChild(IdRef("r2", "0"));
    argList->addChild(IdRef("r3", "4"));

    TestPrinting(refQArgs, argStr);
}

TEST(ASTNodeTests, QOpCreationTest) {
    std::string meaStr = "measure r0[2] -> c2[5];";
    NodeRef refMeasure = NDQOpMeasure::create(IdRef("r0", "2"), IdRef("c2", "5"));
    TestPrinting(refMeasure, meaStr);

    std::string resetStr = "reset r0[5];";
    NodeRef refReset = NDQOpReset::create(IdRef("r0", "5"));
    TestPrinting(refReset, resetStr);

    std::string barrierStr = "barrier r0[2], r1[5], r3, r5;";
    NodeRef refBarrier;
    {
        NodeRef refArgList = NDList::create();
        NDList *list = dynCast<NDList>(refArgList);
        list->addChild(IdRef("r0", "2"));
        list->addChild(IdRef("r1", "5"));
        list->addChild(Id("r3"));
        list->addChild(Id("r5"));
        refBarrier = NDQOpBarrier::create(refArgList);
    }
    TestPrinting(refBarrier, barrierStr);

    std::string genStr = "id r0, r5;";
    NodeRef refGeneric;
    {
        NodeRef refArgList = NDList::create();
        NDList *list = dynCast<NDList>(refArgList);
        list->addChild(Id("r0"));
        list->addChild(Id("r5"));
        refGeneric = NDQOpGeneric::create(Id("id"), NDList::create(), refArgList);
    }
    TestPrinting(refGeneric, genStr);
}

TEST(ASTNodeTests, GOpListCreationTest) {
    std::string gopStr = "id(3) r0;id(5) r5;id(7) r3;";

    NodeRef refGOp = NDGOpList::create();
    NDGOpList* gopList = dynCast<NDGOpList>(refGOp);

    gopList->addChild(IdGate("r0", "3"));
    gopList->addChild(IdGate("r5", "5"));
    gopList->addChild(IdGate("r3", "7"));

    TestPrinting(refGOp, gopStr);
}

TEST(ASTNodeTests, StmtListCreationTest) {
    std::string stmtStr = "id(3) r0;id(5) r5;id(7) r3;";

    NodeRef refStmt = NDStmtList::create();
    NDStmtList* stmtList = dynCast<NDStmtList>(refStmt);

    stmtList->addChild(IdGate("r0", "3"));
    stmtList->addChild(IdGate("r5", "5"));
    stmtList->addChild(IdGate("r3", "7"));

    TestPrinting(refStmt, stmtStr);
}

TEST(ASTNodeTests, OpaqueGateDeclCreationTest) {
    std::string opaqueStr = "opaque Ogate(10, sin(pi), (-3.14159)) r0, r5;";

    NodeRef refQArgs = NDList::create();
    NDList* qaList = dynCast<NDList>(refQArgs);
    qaList->addChild(Id("r0"));
    qaList->addChild(Id("r5"));

    NodeRef refArgs = NDList::create();
    NDList* aList = dynCast<NDList>(refArgs);
    aList->addChild(Int("10"));
    aList->addChild(NDUnaryOp::create(NDUnaryOp::UOP_SIN, Id("pi")));
    aList->addChild(NDUnaryOp::create(NDUnaryOp::UOP_NEG, Real("3.14159")));

    NodeRef refOpaque = NDOpaque::create(Id("Ogate"), refArgs, refQArgs);
    TestPrinting(refOpaque, opaqueStr);
}

TEST(ASTNodeTests, GateDeclCreationTest) {
    std::string idGateStr = "gate id r0 {}";

    NodeRef refQArgs = NDList::create();
    dynCast<NDList>(refQArgs)->addChild(Id("r0"));

    NodeRef refIdGate = NDGateDecl::create(Id("id"), NDList::create(), refQArgs, 
            NDStmtList::create());
    TestPrinting(refIdGate, idGateStr);
}
