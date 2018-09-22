#ifndef __EFD_QMODULE_H__
#define __EFD_QMODULE_H__

#include "enfield/Analysis/Nodes.h"
#include "enfield/Transform/Pass.h"

#include <unordered_map>

namespace efd {
    class Pass;

    /// \brief Qasm module representation.
    class QModule {
        public:
            typedef QModule* Ref;
            typedef std::unique_ptr<QModule> uRef;
            typedef std::shared_ptr<QModule> sRef;

            typedef std::vector<NDInclude::uRef> IncludeVector; 
            typedef std::vector<NDRegDecl::Ref> RegsVector; 
            typedef std::vector<NDGateSign::Ref> GatesVector; 

            typedef std::unordered_map<std::string, NDRegDecl::uRef> RegsMap; 
            typedef std::unordered_map<std::string, NDGateSign::uRef> GatesMap; 

            typedef std::unordered_map<std::string, NDId::Ref> IdMap;
            typedef std::unordered_map<NDGateDecl::Ref, IdMap> GateIdMap;

            typedef Node::Iterator Iterator;
            typedef Node::ConstIterator ConstIterator;

            typedef RegsVector::iterator RegIterator;
            typedef RegsVector::const_iterator RegConstIterator;

            typedef GatesVector::iterator GateIterator;
            typedef GatesVector::const_iterator GateConstIterator;

        private:
            NDQasmVersion::uRef mVersion;
            IncludeVector mIncludes;

            GateIdMap mGateIdMap;
            RegsMap mRegsMap; 
            GatesMap mGatesMap; 

            RegsVector mRegs;
            GatesVector mGates;
            NDStmtList::uRef mStatements;

            QModule();

        public:
            ~QModule();

            /// \brief Gets the qasm version.
            NDQasmVersion::Ref getVersion();
            /// \brief Sets the qasm version.
            void setVersion(NDQasmVersion::uRef version);

            /// \brief Inserts an include node.
            void insertInclude(NDInclude::uRef incl);
            /// \brief Inserts a register declaration node.
            void insertReg(NDRegDecl::uRef reg);

            /// \brief Removes all quantum registers.
            void removeAllQRegs();

            /// \brief Returns a iterator pointing to \p ref, if it exists.
            Iterator findStatement(Node::Ref ref);
            /// \brief Removes the node pointed by \p it.
            void removeStatement(Iterator it);

            /// \brief Inlines \p call and returns an iterator to the first node inserted.
            Iterator inlineCall(NDQOp::Ref call);
            /// \brief Inserts \p ref after \p it, and returns a iterator to this node.
            Iterator insertStatementAfter(Iterator it, Node::uRef ref);
            /// \brief Inserts \p ref before \p it, and returns a iterator to this node.
            Iterator insertStatementBefore(Iterator it, Node::uRef ref);
            /// \brief Inserts \p ref at the beginning, and returns a iterator to this node.
            Iterator insertStatementFront(Node::uRef ref);
            /// \brief Inserts \p ref at the back, and returns a iterator to this node.
            Iterator insertStatementLast(Node::uRef ref);
            Iterator insertStatementLast(std::vector<Node::uRef> stmts);

            /// \brief Replaces the \p stmt by the vector \p stmts.
            Iterator replaceStatement(Node::Ref stmt, std::vector<Node::uRef> stmts);

            /// \brief Removes all statements present int this module.
            void clearStatements();

            /// \brief Inserts a gate to the QModule.
            void insertGate(NDGateSign::uRef gate);

            /// \brief Returns the \p i-th statement.
            Node::Ref getStatement(uint32_t i);

            /// \brief Return the number of registers.
            uint32_t getNumberOfRegs() const;
            /// \brief Return the number of gates.
            uint32_t getNumberOfGates() const;
            /// \brief Return the number of statements.
            uint32_t getNumberOfStmts() const;

            /// \brief Iterator to the beginning of the register node vector.
            RegIterator reg_begin();
            /// \brief ConstIterator to the beginning of the register node vector.
            RegConstIterator reg_begin() const;
            /// \brief Iterator to the end of the register node vector.
            RegIterator reg_end();
            /// \brief ConstIterator to the end of the register node vector.
            RegConstIterator reg_end() const;

            /// \brief Iterator to the beginning of the gate declaration node vector.
            GateIterator gates_begin();
            /// \brief ConstIterator to the beginning of the gate declaration node vector.
            GateConstIterator gates_begin() const;
            /// \brief Iterator to the end of the gate declaration node vector.
            GateIterator gates_end();
            /// \brief ConstIterator to the end of the gate declaration node vector.
            GateConstIterator gates_end() const;

            /// \brief Iterator to the beginning of the statement node vector.
            Iterator stmt_begin();
            /// \brief ConstIterator to the beginning of the statement node vector.
            ConstIterator stmt_begin() const;
            /// \brief Iterator to the end of the statement node vector.
            Iterator stmt_end();
            /// \brief ConstIterator to the end of the statement node vector.
            ConstIterator stmt_end() const;

            /// \brief Order the statements according to \p order.
            void orderby(std::vector<uint32_t> order);

            /// \brief Prints the QModule to a std::ostream.
            void print(std::ostream& O = std::cout, bool pretty = false,
                    bool printGates = false) const;
            /// \brief Returns a std::string representation of the QModule.
            std::string toString(bool pretty = false, bool printGates = false) const;

            /// \brief Gets the quantum variable mapped to \p id from some gate.
            Node::Ref getQVar(std::string id, NDGateDecl::Ref gate = nullptr) const;
            /// \brief Returns true if there is a quantum variable \p id inside gate
            /// \p gate (if not null).
            bool hasQVar(std::string id, NDGateDecl::Ref gate = nullptr) const;
            /// \brief Gets the quantum gate mapped to \p id.
            NDGateSign::Ref getQGate(std::string id) const;
            /// \brief Returns true if there is a quantum gate \p id.
            bool hasQGate(std::string id) const;

            /// \brief Clones the current qmodule.
            uRef clone() const;

            /// \brief Create a new empty QModule.
            static uRef Create();
            /// \brief Process the AST in order to obtain the QModule.
            static uRef GetFromAST(Node::uRef ref);
            /// \brief Parses the file \p filename and returns a QModule.
            static uRef Parse(std::string filename, std::string path = "./");
            /// \brief Parses the string \p program and returns a QModule.
            static uRef ParseString(std::string program);
    };
}

#endif
