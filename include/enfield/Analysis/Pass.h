#ifndef __EFD_PASS_H__
#define __EFD_PASS_H__

#include "enfield/Analysis/NodeVisitor.h"

namespace efd {
    /// \brief Base class for implementation of QModule passes.
    /// This information will be used when the QModule's function
    /// is called.
    class Pass : public NodeVisitor {
        public:
            enum Kind {
                K_GATE_PASS = 1,
                K_REG_DECL_PASS = 2,
                K_STMT_PASS = 4,
                K_AST_PASS = 8
            };

        private:
            bool mApplied;

        protected:
            unsigned mUK;
            virtual void initImpl();

        public:
            Pass();

            bool isGatePass() const;
            bool isRegDeclPass() const;
            bool isStatementPass() const;
            bool isASTPass() const;

            bool wasApplied() const;

            void init();
    };
};

#endif
