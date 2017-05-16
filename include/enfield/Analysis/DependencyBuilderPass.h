#ifndef __EFD_DEPENDENCY_BUILDER_PASS_H__
#define __EFD_DEPENDENCY_BUILDER_PASS_H__

#include "enfield/Analysis/Nodes.h"
#include "enfield/Analysis/NodeVisitor.h"
#include "enfield/Analysis/QModule.h"

#include <unordered_map>
#include <vector>
#include <set>

namespace efd {

    class QbitToNumberPass : public NodeVisitor {
        private:
            typedef std::vector<std::string> QbitMap;

            std::unordered_map<NDGateDecl*, QbitMap> mLIdMap;
            QbitMap mGIdMap;

            QbitToNumberPass();

        public:
            void visit(NDDecl* ref) override;
            void visit(NDGateDecl* ref) override;

            /// \brief Returns an unsigned number representing the id
            /// in this specific gate (if any).
            unsigned getUId(std::string id, NDGateDecl* gateRef = nullptr) const;

            /// \brief Returns the std::string id representation of the
            /// corresponding unsigned id in the specific gate (if any).
            std::string getStrId(unsigned id, NDGateDecl* gateRef = nullptr) const;

            /// \brief Returns a new instance of this class.
            static QbitToNumberPass* create();
    };

    class DependencyBuilderPass : public NodeVisitor {
        private:
            typedef std::vector<std::set<unsigned>> DepsSet;

            QModule* mMod;
            NDGateDecl* mCurrentGate;
            QbitToNumberPass* mQbitMap;

            std::unordered_map<NDGateDecl*, DepsSet> mLDeps;
            DepsSet mGDeps;

            DependencyBuilderPass(QModule* mod, QbitToNumberPass* pass = nullptr);

            /// \brief Gets an unsingned id for \p ref.
            unsigned getUId(NodeRef ref);
            /// \brief Gets the DepsSet corresponding to the current quantum gate,
            /// or the global (if current gate is null).
            DepsSet* getCurrentDepsSet();

        public:
            void visit(NDGateDecl* ref) override;
            void visit(NDGOpList* ref) override;
            void visit(NDQOpCX* ref) override;
            void visit(NDQOpGeneric* ref) override;

            void init() override;

            /// \brief Returns a new instance of this class.
            static DependencyBuilderPass* create(QModule* mod, QbitToNumberPass* pass = nullptr);
    };
};

#endif
