#ifndef __EFD_DEPENDENCY_BUILDER_H__
#define __EFD_DEPENDENCY_BUILDER_H__

#include "enfield/Analysis/Nodes.h"
#include "enfield/Analysis/NodeVisitor.h"

#include <unordered_map>
#include <vector>
#include <set>

namespace efd {

    class IdToNumberPass : public NodeVisitor {
        private:
            typedef std::vector<std::string> IdMap;
            std::unordered_map<NDGateDecl*, IdMap> mLIdMap;
            IdMap mGIdMap;

        public:
    };

    class DependencyBuilderPass : public NodeVisitor {
        private:
            NDGateDecl* mCurrentGate;
            std::unordered_map<NDGateDecl*, std::vector<std::set<int>>> mDeps;

        public:
            void visit(NDGateDecl* ref) override;
            void visit(NDGOpList* ref) override;
            void visit(NDQOpCX* ref) override;
    };
};

#endif
