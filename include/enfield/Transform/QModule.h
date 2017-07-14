#ifndef __EFD_QMODULE_H__
#define __EFD_QMODULE_H__

#include "enfield/Analysis/Nodes.h"
#include "enfield/Transform/IdTable.h"
#include "enfield/Transform/QModulefyPass.h"

#include <unordered_map>

namespace efd {
    class Pass;

    /// \brief Qasm module representation.
    class QModule {
        public:
            typedef QModule* Ref;
            typedef std::unique_ptr<QModule> uRef;
            typedef std::shared_ptr<QModule> sRef;

            typedef Node::Iterator Iterator;
            typedef Node::ConstIterator ConstIterator;

            typedef std::vector<Node::Ref>::iterator NodeIterator;
            typedef std::vector<Node::Ref>::const_iterator NodeConstIterator;

            typedef std::vector<NDDecl::Ref>::iterator RegIterator;
            typedef std::vector<NDDecl::Ref>::const_iterator RegConstIterator;

            typedef std::vector<Node::Ref>::iterator GateIterator;
            typedef std::vector<Node::Ref>::const_iterator GateConstIterator;

        private:
            Node::uRef mAST;
            Node::Ref mVersion;
            NDStmtList::Ref mStmtList;

            std::vector<NDDecl::Ref> mRegDecls;
            std::vector<Node::Ref> mGates;
            std::vector<Node::Ref> mStatements;

            IdTable::uRef mTable;
            std::unordered_map<Node::Ref, IdTable::uRef> mIdTableMap;

            bool mValid;
            bool mStdLibsParsed;
            QModulefyPass::sRef mQModulefy;

            QModule(Node::uRef ref);

            /// \brief Registers the swap gate in the module.
            void registerSwapGate(Iterator it);

        public:
            /// \brief Gets the qasm version.
            Node::Ref getVersion();

            /// \brief Replace all quantum registers with this sequence of declarations.
            /// 
            /// This should be used when renaming registers to the architecture's register
            /// set.
            void replaceAllRegsWith(std::vector<NDDecl::uRef> newRegs);

            /// \brief Inlines \p call and returns an iterator to the first node inserted.
            Iterator inlineCall(NDQOpGeneric::Ref call);
            /// \brief Inserts \p ref after \p it, and returns a iterator to this node.
            Iterator insertNodeAfter(Iterator it, Node::uRef ref);
            /// \brief Inserts \p ref before \p it, and returns a iterator to this node.
            Iterator insertNodeBefore(Iterator it, Node::uRef ref);
            /// \brief Inserts a swap call between \p lhs and \p rhs, before the iterator
            /// location. Returns a iterator to the first swap instruction.
            Iterator insertSwapAfter(Iterator it, Node::Ref lhs, Node::Ref rhs);
            /// \brief Inserts a swap call between \p lhs and \p rhs, after the iterator
            /// location. Returns the iterator on the same position.
            Iterator insertSwapBefore(Iterator it, Node::Ref lhs, Node::Ref rhs);

            /// \brief Inserts a node at the last position.
            void insertStatementLast(Node::uRef node);

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
            NodeIterator stmt_begin();
            /// \brief ConstIterator to the beginning of the statement node vector.
            NodeConstIterator stmt_begin() const;
            /// \brief Iterator to the end of the statement node vector.
            NodeIterator stmt_end();
            /// \brief ConstIterator to the end of the statement node vector.
            NodeConstIterator stmt_end() const;

            /// \brief Prints the QModule to a std::ostream.
            void print(std::ostream& O = std::cout, bool pretty = false) const;
            /// \brief Returns a std::string representation of the QModule.
            std::string toString(bool pretty = false) const;

            /// \brief Gets the mapped IdTable.
            IdTable::Ref getIdTable(NDGateDecl::Ref ref);

            /// \brief Gets the quantum variable mapped to \p id from some gate.
            Node::Ref getQVar(std::string id, NDGateDecl* gate = nullptr, bool recursive = true);
            /// \brief Gets the quantum gate mapped to \p id.
            NDGateDecl* getQGate(std::string id, bool recursive = true);

            /// \brief Invalidates the current QModule.
            void invalidate();
            /// \brief Validates the current QModule. This means that the information that
            /// is in its properties are valid.
            void validate();
            /// \brief Returns true if this QModule is valid.
            bool isValid() const;

            /// \brief Applies the pass in the QModule. If the pass has already been applied,
            /// it won't be applied again unless \p force is set.
            void runPass(Pass::Ref pass, bool force = false);

            /// \brief Clones the current qmodule.
            std::unique_ptr<QModule> clone() const;

            /// \brief Create a new empty QModule.
            static std::unique_ptr<QModule> Create(bool forceStdLib = true);
            /// \brief Process the AST in order to obtain the QModule.
            static std::unique_ptr<QModule> GetFromAST(Node::uRef ref);
            /// \brief Parses the file \p filename and returns a QModule.
            static std::unique_ptr<QModule> Parse(std::string filename,
                    std::string path = "./", bool forceStdLib = true);
            /// \brief Parses the string \p program and returns a QModule.
            static std::unique_ptr<QModule> ParseString(std::string program,
                    bool forceStdLib = true);

            friend class QModulefyPass;

    };
}

#endif
