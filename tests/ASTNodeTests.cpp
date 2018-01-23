
#include "gtest/gtest.h"

#include "enfield/Analysis/Nodes.h"
#include "enfield/Support/RTTI.h"

#include <string>

using namespace efd;

static NDId::uRef Id(std::string s);
static NDIdRef::uRef IdRef(std::string id, std::string sz);
static NDQOpGen::uRef IdGate(std::string id, std::string par);
static NDInt::uRef Int(std::string s);
static NDReal::uRef Real(std::string s);

static NDId::uRef Id(std::string s) {
    return NDId::Create(s);
}

static NDIdRef::uRef IdRef(std::string id, std::string sz) {
    return NDIdRef::Create(Id(id), Int(sz));
}

static NDQOpGen::uRef IdGate(std::string id, std::string par) {
    auto refQAL = NDList::Create();
    auto refAL = NDList::Create();
    refQAL->addChild(Id(id));
    refAL->addChild(Int(par));
    return NDQOpGen::Create(Id("id"), std::move(refAL), std::move(refQAL));
}

static NDInt::uRef Int(std::string s) {
    return NDInt::Create(s);
}

static NDReal::uRef Real(std::string s) {
    return NDReal::Create(s);
}

static void TestPrinting(Node::Ref node, std::string rhs) {
    ASSERT_FALSE(node == nullptr);
    ASSERT_EQ(node->toString(), rhs);

    std::ostringstream ss;
    node->print(ss);
    ASSERT_EQ(ss.str(), rhs);
}

static void TestFind(Node::Ref ref) {
    for (uint32_t i = 0, e = ref->getChildNumber(); i < e; ++i) {
        Node::Ref child = ref->getChild(i);

        ASSERT_TRUE(child->getParent() == ref);
        ASSERT_FALSE(ref->findChild(child) == ref->end());
        TestFind(child);
    }
}

static void TestEqual(Node::Ref ref) {
    ref->equals(ref);
}

TEST(ASTNodeTests, LiteralCreationTest) {
    std::string idStr = "r0";
    auto refId = NDId::Create(idStr);
    TestPrinting(refId.get(), idStr);
    TestFind(refId.get());

    std::string intStr = "10";
    auto refInt = NDInt::Create(intStr);
    TestPrinting(refInt.get(), intStr);
    TestFind(refInt.get());

    std::string dStr = "3.14159";
    auto refDv = NDReal::Create(dStr);
    TestPrinting(refDv.get(), dStr);
    TestFind(refDv.get());

    std::string rStr = "r0[3]";
    auto refIdRef = NDIdRef::Create(Id("r0"), Int("3"));
    TestPrinting(refIdRef.get(), rStr);
    TestFind(refIdRef.get());
}

TEST(ASTNodeTests, BinOpCreationTest) {
    std::string addStr = "(pi + 5)";
    auto refAdd = NDBinOp::Create(NDBinOp::OP_ADD, Id("pi"), Int("5"));
    TestPrinting(refAdd.get(), addStr);
    TestFind(refAdd.get());

    std::string subStr = "(pi - 5)";
    auto refSub = NDBinOp::Create(NDBinOp::OP_SUB, Id("pi"), Int("5"));
    TestPrinting(refSub.get(), subStr);
    TestFind(refSub.get());

    std::string mulStr = "(pi * 5)";
    auto refMul = NDBinOp::Create(NDBinOp::OP_MUL, Id("pi"), Int("5"));
    TestPrinting(refMul.get(), mulStr);
    TestFind(refMul.get());

    std::string divStr = "(pi / 5)";
    auto refDiv = NDBinOp::Create(NDBinOp::OP_DIV, Id("pi"), Int("5"));
    TestPrinting(refDiv.get(), divStr);
    TestFind(refDiv.get());

    std::string powStr = "(pi ^ 5)";
    auto refPow = NDBinOp::Create(NDBinOp::OP_POW, Id("pi"), Int("5"));
    TestPrinting(refPow.get(), powStr);
    TestFind(refPow.get());

    std::string mixStr = "((pi ^ 5) / (2 * 5.8))";
    auto refLhs = NDBinOp::CreatePow(Id("pi"), Int("5"));
    auto refRhs = NDBinOp::CreateMul(Int("2"), Real("5.8"));
    auto refMix = NDBinOp::CreateDiv(std::move(refLhs), std::move(refRhs));
    TestPrinting(refMix.get(), mixStr);
    TestFind(refMix.get());
}

TEST(ASTNodeTests, UnaryOpCreationTest) {
    std::string sinStr = "sin(pi)";
    auto refSin = NDUnaryOp::Create(NDUnaryOp::UOP_SIN, Id("pi"));
    TestPrinting(refSin.get(), sinStr);
    TestFind(refSin.get());

    std::string cosStr = "cos(pi)";
    auto refCos = NDUnaryOp::Create(NDUnaryOp::UOP_COS, Id("pi"));
    TestPrinting(refCos.get(), cosStr);
    TestFind(refCos.get());

    std::string tanStr = "tan(pi)";
    auto refTan = NDUnaryOp::Create(NDUnaryOp::UOP_TAN, Id("pi"));
    TestPrinting(refTan.get(), tanStr);
    TestFind(refTan.get());

    std::string lnStr = "ln(pi)";
    auto refLn = NDUnaryOp::Create(NDUnaryOp::UOP_LN, Id("pi"));
    TestPrinting(refLn.get(), lnStr);
    TestFind(refLn.get());

    std::string negStr = "(-pi)";
    auto refNeg = NDUnaryOp::Create(NDUnaryOp::UOP_NEG, Id("pi"));
    TestPrinting(refNeg.get(), negStr);
    TestFind(refNeg.get());

    std::string expStr = "exp(pi)";
    auto refExp = NDUnaryOp::Create(NDUnaryOp::UOP_EXP, Id("pi"));
    TestPrinting(refExp.get(), expStr);
    TestFind(refExp.get());

    std::string sqrtStr = "sqrt(pi)";
    auto refSqrt = NDUnaryOp::Create(NDUnaryOp::UOP_SQRT, Id("pi"));
    TestPrinting(refSqrt.get(), sqrtStr);
    TestFind(refSqrt.get());
}

TEST(ASTNodeTests, DeclCreationTest) {
    std::string cStr = "creg r0[5];";
    auto refC = NDRegDecl::CreateC(Id("r0"), Int("5"));
    TestPrinting(refC.get(), cStr);
    TestFind(refC.get());

    std::string qStr = "qreg r0[5];";
    auto refQ = NDRegDecl::CreateQ(Id("r0"), Int("5"));
    TestPrinting(refQ.get(), qStr);
    TestFind(refQ.get());
}

TEST(ASTNodeTests, ListCreationTest) {
    std::string argStr = "r0, r1, r2[0], r3[4]";

    auto argList = NDList::Create();

    argList->addChild(Id("r0"));
    argList->addChild(Id("r1"));
    argList->addChild(IdRef("r2", "0"));
    argList->addChild(IdRef("r3", "4"));

    TestPrinting(argList.get(), argStr);
    TestFind(argList.get());
}

TEST(ASTNodeTests, QOpCreationTest) {
    std::string meaStr = "measure r0[2] -> c2[5];";
    auto refMeasure = NDQOpMeasure::Create(IdRef("r0", "2"), IdRef("c2", "5"));
    TestPrinting(refMeasure.get(), meaStr);
    TestFind(refMeasure.get());

    std::string resetStr = "reset r0[5];";
    auto refReset = NDQOpReset::Create(IdRef("r0", "5"));
    TestPrinting(refReset.get(), resetStr);
    TestFind(refReset.get());

    std::string barrierStr = "barrier r0[2], r1[5], r3, r5;";
    NDQOpBarrier::uRef refBarrier;
    {
        auto refArgList = NDList::Create();
        refArgList->addChild(IdRef("r0", "2"));
        refArgList->addChild(IdRef("r1", "5"));
        refArgList->addChild(Id("r3"));
        refArgList->addChild(Id("r5"));
        refBarrier = NDQOpBarrier::Create(std::move(refArgList));
    }
    TestPrinting(refBarrier.get(), barrierStr);
    TestFind(refBarrier.get());

    std::string genStr = "id r0, r5;";
    NDQOpGen::uRef refGeneric;
    {
        auto refArgList = NDList::Create();
        refArgList->addChild(Id("r0"));
        refArgList->addChild(Id("r5"));
        refGeneric = NDQOpGen::Create(Id("id"), NDList::Create(), std::move(refArgList));
    }
    TestPrinting(refGeneric.get(), genStr);
    TestFind(refGeneric.get());
}

TEST(ASTNodeTests, GOpListCreationTest) {
    std::string gopStr = "id(3) r0;id(5) r5;id(7) r3;";

    auto refGOp = NDGOpList::Create();

    refGOp->addChild(IdGate("r0", "3"));
    refGOp->addChild(IdGate("r5", "5"));
    refGOp->addChild(IdGate("r3", "7"));

    TestPrinting(refGOp.get(), gopStr);
    TestFind(refGOp.get());
}

TEST(ASTNodeTests, StmtListCreationTest) {
    std::string stmtStr = "id(3) r0;id(5) r5;id(7) r3;";

    auto refStmt = NDStmtList::Create();

    refStmt->addChild(IdGate("r0", "3"));
    refStmt->addChild(IdGate("r5", "5"));
    refStmt->addChild(IdGate("r3", "7"));

    TestPrinting(refStmt.get(), stmtStr);
    TestFind(refStmt.get());
}

TEST(ASTNodeTests, OpaqueGateDeclCreationTest) {
    std::string opaqueStr = "opaque Ogate(10, sin(pi), (-3.14159)) r0, r5;";

    auto refQArgs = NDList::Create();
    refQArgs->addChild(Id("r0"));
    refQArgs->addChild(Id("r5"));

    auto refArgs = NDList::Create();
    refArgs->addChild(Int("10"));
    refArgs->addChild(NDUnaryOp::Create(NDUnaryOp::UOP_SIN, Id("pi")));
    refArgs->addChild(NDUnaryOp::Create(NDUnaryOp::UOP_NEG, Real("3.14159")));

    auto refOpaque = NDOpaque::Create(Id("Ogate"), std::move(refArgs), std::move(refQArgs));
    TestPrinting(refOpaque.get(), opaqueStr);
    TestFind(refOpaque.get());
}

TEST(ASTNodeTests, GateDeclCreationTest) {
    std::string idGateStr = "gate id r0 {}";

    auto refQArgs = NDList::Create();
    refQArgs->addChild(Id("r0"));

    auto refIdGate = NDGateDecl::Create(Id("id"), NDList::Create(),
            std::move(refQArgs), NDGOpList::Create());
    TestPrinting(refIdGate.get(), idGateStr);
    TestFind(refIdGate.get());
}

TEST(ASTNodeTests, IfStmtCreationTest) {
    std::string ifStr = "if (someid == 1) reset otherid[0];";

    auto refReset = NDQOpReset::Create(NDIdRef::Create(Id("otherid"), Int("0")));
    auto refIf = NDIfStmt::Create(Id("someid"), Int("1"), std::move(refReset));
    TestPrinting(refIf.get(), ifStr);
    TestFind(refIf.get());
}

TEST(ASTNodeTests, IncludeCreationTest) {
    std::string includeStr = "include \"files/_qelib1.inc\";";

    auto refInclude = NDInclude::Create(Id("files/_qelib1.inc"), NDStmtList::Create());
    TestPrinting(refInclude.get(), includeStr);
    TestFind(refInclude.get());
}

// --------------------------------------
// ------------ EqualTests --------------
// --------------------------------------

TEST(ASTNodeTests, LiteralEqualTest) {
    auto refId = NDId::Create(std::string("r0"));
    TestEqual(refId.get());

    auto refInt = NDInt::Create(std::string("10"));
    TestEqual(refInt.get());

    auto refDv = NDReal::Create(std::string("3.14159"));
    TestEqual(refDv.get());

    auto refIdRef = NDIdRef::Create(Id("r0"), Int("3"));
    TestEqual(refIdRef.get());
}

TEST(ASTNodeTests, BinOpEqualTest) {
    auto refAdd = NDBinOp::Create(NDBinOp::OP_ADD, Id("pi"), Int("5"));
    TestEqual(refAdd.get());

    auto refSub = NDBinOp::Create(NDBinOp::OP_SUB, Id("pi"), Int("5"));
    TestEqual(refSub.get());

    auto refMul = NDBinOp::Create(NDBinOp::OP_MUL, Id("pi"), Int("5"));
    TestEqual(refMul.get());

    auto refDiv = NDBinOp::Create(NDBinOp::OP_DIV, Id("pi"), Int("5"));
    TestEqual(refDiv.get());

    auto refPow = NDBinOp::Create(NDBinOp::OP_POW, Id("pi"), Int("5"));
    TestEqual(refPow.get());

    auto refLhs = NDBinOp::CreatePow(Id("pi"), Int("5"));
    auto refRhs = NDBinOp::CreateMul(Int("2"), Real("5.8"));
    auto refMix = NDBinOp::CreateDiv(std::move(refLhs), std::move(refRhs));
    TestEqual(refMix.get());
}

TEST(ASTNodeTests, UnaryOpEqualTest) {
    auto refSin = NDUnaryOp::Create(NDUnaryOp::UOP_SIN, Id("pi"));
    TestEqual(refSin.get());

    auto refCos = NDUnaryOp::Create(NDUnaryOp::UOP_COS, Id("pi"));
    TestEqual(refCos.get());

    auto refTan = NDUnaryOp::Create(NDUnaryOp::UOP_TAN, Id("pi"));
    TestEqual(refTan.get());

    auto refLn = NDUnaryOp::Create(NDUnaryOp::UOP_LN, Id("pi"));
    TestEqual(refLn.get());

    auto refNeg = NDUnaryOp::Create(NDUnaryOp::UOP_NEG, Id("pi"));
    TestEqual(refNeg.get());

    auto refExp = NDUnaryOp::Create(NDUnaryOp::UOP_EXP, Id("pi"));
    TestEqual(refExp.get());

    auto refSqrt = NDUnaryOp::Create(NDUnaryOp::UOP_SQRT, Id("pi"));
    TestEqual(refSqrt.get());
}

TEST(ASTNodeTests, DeclEqualTest) {
    auto refC = NDRegDecl::CreateC(Id("r0"), Int("5"));
    TestEqual(refC.get());

    auto refQ = NDRegDecl::CreateQ(Id("r0"), Int("5"));
    TestEqual(refQ.get());
}

TEST(ASTNodeTests, ListEqualTest) {
    auto argList = NDList::Create();

    argList->addChild(Id("r0"));
    argList->addChild(Id("r1"));
    argList->addChild(IdRef("r2", "0"));
    argList->addChild(IdRef("r3", "4"));

    TestEqual(argList.get());
}

TEST(ASTNodeTests, QOpEqualTest) {
    auto refMeasure = NDQOpMeasure::Create(IdRef("r0", "2"), IdRef("c2", "5"));
    TestEqual(refMeasure.get());

    auto refReset = NDQOpReset::Create(IdRef("r0", "5"));
    TestEqual(refReset.get());

    NDQOpBarrier::uRef refBarrier;
    {
        auto refArgList = NDList::Create();
        refArgList->addChild(IdRef("r0", "2"));
        refArgList->addChild(IdRef("r1", "5"));
        refArgList->addChild(Id("r3"));
        refArgList->addChild(Id("r5"));
        refBarrier = NDQOpBarrier::Create(std::move(refArgList));
    }
    TestEqual(refBarrier.get());

    NDQOpGen::uRef refGeneric;
    {
        auto refArgList = NDList::Create();
        refArgList->addChild(Id("r0"));
        refArgList->addChild(Id("r5"));
        refGeneric = NDQOpGen::Create(Id("id"), NDList::Create(), std::move(refArgList));
    }
    TestEqual(refGeneric.get());
}

TEST(ASTNodeTests, GOpListEqualTest) {
    auto refGOp = NDGOpList::Create();

    refGOp->addChild(IdGate("r0", "3"));
    refGOp->addChild(IdGate("r5", "5"));
    refGOp->addChild(IdGate("r3", "7"));

    TestEqual(refGOp.get());
}

TEST(ASTNodeTests, StmtListEqualTest) {
    auto refStmt = NDStmtList::Create();

    refStmt->addChild(IdGate("r0", "3"));
    refStmt->addChild(IdGate("r5", "5"));
    refStmt->addChild(IdGate("r3", "7"));

    TestEqual(refStmt.get());
}

TEST(ASTNodeTests, OpaqueGateDeclEqualTest) {
    auto refQArgs = NDList::Create();
    refQArgs->addChild(Id("r0"));
    refQArgs->addChild(Id("r5"));

    auto refArgs = NDList::Create();
    refArgs->addChild(Int("10"));
    refArgs->addChild(NDUnaryOp::Create(NDUnaryOp::UOP_SIN, Id("pi")));
    refArgs->addChild(NDUnaryOp::Create(NDUnaryOp::UOP_NEG, Real("3.14159")));

    auto refOpaque = NDOpaque::Create(Id("Ogate"), std::move(refArgs), std::move(refQArgs));
    TestEqual(refOpaque.get());
}

TEST(ASTNodeTests, GateDeclEqualTest) {
    auto refQArgs = NDList::Create();
    refQArgs->addChild(Id("r0"));

    auto refIdGate = NDGateDecl::Create(Id("id"), NDList::Create(),
            std::move(refQArgs), NDGOpList::Create());
    TestEqual(refIdGate.get());
}

TEST(ASTNodeTests, IfStmtEqualTest) {
    auto refReset = NDQOpReset::Create(NDIdRef::Create(Id("otherid"), Int("0")));
    auto refIf = NDIfStmt::Create(Id("someid"), Int("1"), std::move(refReset));
    TestEqual(refIf.get());
}

TEST(ASTNodeTests, IncludeEqualTest) {
    auto refInclude = NDInclude::Create(Id("files/_qelib1.inc"), NDStmtList::Create());
    TestEqual(refInclude.get());
}
