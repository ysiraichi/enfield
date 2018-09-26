#ifndef __EFD_QUBIT_REMAP_PASS_H__
#define __EFD_QUBIT_REMAP_PASS_H__

#include "enfield/Transform/XbitToNumberPass.h"
#include "enfield/Analysis/NodeVisitor.h"
#include "enfield/Support/Defs.h"

namespace efd {
    /// \brief Visitor that remaps the qubits according to a specific \em Mapping.
    class QubitRemapVisitor : public NodeVisitor {
        private:
            const Mapping& mMap;
            const XbitToNumber& mXtoN;
            bool mWasReplaced;
    
        public:
            QubitRemapVisitor(const Mapping& m, const XbitToNumber& xtoN);

            /// \brief Returns whether the last call to \em visitNDQOp ended up
            /// in replacing all qargs.
            bool wasReplaced();
    
            /// \brief Every \em visit function calls this function.
            ///
            /// It replaces the old qubit nodes by nodes corresponding by its
            /// respectively mapped qubits.
            void visitNDQOp(NDQOp::Ref qop);
    
            void visit(NDQOpMeasure::Ref ref) override;
            void visit(NDQOpReset::Ref ref) override;
            void visit(NDQOpU::Ref ref) override;
            void visit(NDQOpCX::Ref ref) override;
            void visit(NDQOpBarrier::Ref ref) override;
            void visit(NDQOpGen::Ref ref) override;
            void visit(NDIfStmt::Ref ref) override;
    };

    /// \brief Remaps the qubits in the \em QModule according to a \em Mapping.
    class QubitRemapPass : public PassT<void> {
        public:
            typedef QubitRemapPass* Ref;
            typedef std::unique_ptr<QubitRemapPass> uRef;

            static uint8_t ID;

        private:
            const Mapping& mMap;

        public:
            QubitRemapPass(const Mapping& m);

            bool run(QModule* qmod) override;
            static uRef Create(const Mapping& m);
    };
}

#endif
