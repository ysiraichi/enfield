#include "enfield/Transform/ReverseEdgesPass.h"
#include "enfield/Transform/Utils.h"
#include "enfield/Analysis/NodeVisitor.h"
#include "enfield/Support/RTTI.h"
#include "enfield/Support/uRefCast.h"


uint8_t efd::ReverseEdgesPass::ID = 0;

namespace efd {
    class ReverseEdgesVisitor : public NodeVisitor {
        private:
            ArchGraph::sRef mG;

            /// \brief Inserts this triple into \em mRevVector.
            ///
            /// This also checks if it is inside an if statement.
            void insertIntoRevVector(Node::Ref ref, Node::Ref lhs, Node::Ref rhs);

        public:
            std::vector<std::pair<Node::Ref, Node::uRef>> mRevVector;
            ReverseEdgesVisitor(ArchGraph::sRef g) : mG(g) {}

            void visit(NDQOpCX::Ref ref) override;
            void visit(NDQOpGen::Ref ref) override;
            void visit(NDIfStmt::Ref ref) override;
    };
}

void efd::ReverseEdgesVisitor::insertIntoRevVector
(Node::Ref ref, Node::Ref lhs, Node::Ref rhs) {
    Node::uRef revcall = efd::CreateIRevCX(lhs->clone(), rhs->clone());

    auto parent = dynCast<NDIfStmt>(ref->getParent());
    if (parent != nullptr) {
        auto ifrevcall = uniqueCastForward<NDIfStmt>(parent->clone());
        ifrevcall->setQOp(uniqueCastForward<NDQOp>(std::move(revcall)));

        revcall.reset(ifrevcall.release());
        ref = parent;
    }

    mRevVector.push_back(std::make_pair(ref, std::move(revcall)));
}

void efd::ReverseEdgesVisitor::visit(NDQOpCX::Ref ref) {
    uint32_t uidLhs = mG->getUId(ref->getLhs()->toString());
    uint32_t uidRhs = mG->getUId(ref->getRhs()->toString());

    if (!mG->hasEdge(uidLhs, uidRhs)) {
        insertIntoRevVector(ref, ref->getLhs(), ref->getRhs());
    }
}

void efd::ReverseEdgesVisitor::visit(NDQOpGen::Ref ref) {
    if (ref->getId()->getVal() == "cx") {
        // Have to come up a way to overcome this.
        EfdAbortIf(ref->getQArgs()->getChildNumber() != 2,
                   "CNot gate malformed: `" << ref->toString(false) << "`.");

        NDList* qargs = ref->getQArgs();
        uint32_t uidLhs = mG->getUId(qargs->getChild(0)->toString());
        uint32_t uidRhs = mG->getUId(qargs->getChild(1)->toString());

        if (!mG->hasEdge(uidLhs, uidRhs)) {
            insertIntoRevVector(ref, qargs->getChild(0), qargs->getChild(1));
        }
    }
}

void efd::ReverseEdgesVisitor::visit(NDIfStmt::Ref ref) {
    ref->getQOp()->apply(this);
}

efd::ReverseEdgesPass::ReverseEdgesPass(ArchGraph::sRef graph) : mG(graph) {
}

bool efd::ReverseEdgesPass::run(QModule::Ref qmod) {
    ReverseEdgesVisitor visitor(mG);

    for (auto it = qmod->stmt_begin(), e = qmod->stmt_end(); it != e; ++it) {
        (*it)->apply(&visitor);
    }

    for (auto& pair : visitor.mRevVector) {
        std::vector<Node::uRef> repl;
        repl.push_back(std::move(pair.second));
        qmod->replaceStatement(pair.first, std::move(repl));
    }

    if (visitor.mRevVector.empty()) return false;
    else return true;
}

efd::ReverseEdgesPass::uRef efd::ReverseEdgesPass::Create(ArchGraph::sRef graph) {
    return uRef(new ReverseEdgesPass(graph));
}
