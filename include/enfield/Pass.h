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
            /// \brief Shows whether this pass was already applied.
            bool mApplied;

        protected:
            /// \brief A combination of \p Kinds that represents where this pass
            /// should be run.
            unsigned mUK;
            /// \brief The implementation that should be overrided if the child pass
            /// has to set any variables before running the pass.
            virtual void initImpl();

            Pass();

        public:
            /// \brief Returns true if this pass should be applied on gate declarations.
            bool isGatePass() const;
            /// \brief Returns true if this pass should be applied on register 
            /// declarations.
            bool isRegDeclPass() const;
            /// \brief Returns true if this pass should be applied on statements.
            bool isStatementPass() const;
            /// \brief Returns true if this pass should be applied on the whole AST.
            bool isASTPass() const;

            /// \brief Returns true if the pass was already applied.
            bool wasApplied() const;

            /// \brief Initializes the pass. Note that this function calls \em initImpl.
            void init();

            /// \brief Returns true if the pass invalidates the module.
            virtual bool doesInvalidatesModule() const;
    };
};

#endif
