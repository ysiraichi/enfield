#ifndef __EFD_QMODULE_H__
#define __EFD_QMODULE_H__

#include "enfield/Analysis/Nodes.h"
#include "enfield/Transform/IdTable.h"

#include <unordered_map>

namespace efd {
    class QModulefyPass;
    class Pass;

    /// \brief Qasm module representation.
    class QModule {
        public:
            typedef Node::Iterator Iterator;
            typedef Node::ConstIterator ConstIterator;

        private:
            NodeRef mAST;
            NodeRef mVersion;

            std::vector<NodeRef> mRegDecls;
            std::vector<NodeRef> mGates;
            std::vector<NodeRef> mStatements;

            IdTable mTable;
            std::unordered_map<NodeRef, IdTable> mIdTableMap;

            bool mValid;
            bool mStdLibsParsed;
            QModulefyPass* mQModulefy;

            QModule(NodeRef ref);

            /// \brief Registers the swap gate in the module.
            void registerSwapGate(Iterator it);

        public:
            /// \brief Gets the qasm version.
            NodeRef getVersion();

            /// \brief Replace all quantum registers with this sequence of declarations.
            /// 
            /// This should be used when renaming registers to the architecture's register
            /// set.
            void replaceAllRegsWith(std::vector<NDDecl*> newRegs);

            /// \brief Inlines \p call and returns an iterator to the first node inserted.
            Iterator inlineCall(NDQOpGeneric* call);
            /// \brief Inserts \p ref after \p it, and returns a iterator to this node.
            Iterator insertNodeAfter(Iterator it, NodeRef ref);
            /// \brief Inserts \p ref before \p it, and returns a iterator to this node.
            Iterator insertNodeBefore(Iterator it, NodeRef ref);
            /// \brief Inserts a swap call between \p lhs and \p rhs, before the iterator location. 
            /// Returns a iterator to the first swap instruction.
            Iterator insertSwapAfter(Iterator it, NodeRef lhs, NodeRef rhs);
            /// \brief Inserts a swap call between \p lhs and \p rhs, after the iterator location. 
            /// Returns the iterator on the same position.
            Iterator insertSwapBefore(Iterator it, NodeRef lhs, NodeRef rhs);

            /// \brief Iterator to the beginning of the register node vector.
            Iterator reg_begin();
            /// \brief ConstIterator to the beginning of the register node vector.
            ConstIterator reg_begin() const;
            /// \brief Iterator to the end of the register node vector.
            Iterator reg_end();
            /// \brief ConstIterator to the end of the register node vector.
            ConstIterator reg_end() const;

            /// \brief Iterator to the beginning of the gate declaration node vector.
            Iterator gates_begin();
            /// \brief ConstIterator to the beginning of the gate declaration node vector.
            ConstIterator gates_begin() const;
            /// \brief Iterator to the end of the gate declaration node vector.
            Iterator gates_end();
            /// \brief ConstIterator to the end of the gate declaration node vector.
            ConstIterator gates_end() const;

            /// \brief Iterator to the beginning of the statement node vector.
            Iterator stmt_begin();
            /// \brief ConstIterator to the beginning of the statement node vector.
            ConstIterator stmt_begin() const;
            /// \brief Iterator to the end of the statement node vector.
            Iterator stmt_end();
            /// \brief ConstIterator to the end of the statement node vector.
            ConstIterator stmt_end() const;

            /// \brief Prints the QModule to a std::ostream.
            void print(std::ostream& O = std::cout, bool pretty = false) const;
            /// \brief Returns a std::string representation of the QModule.
            std::string toString(bool pretty = false) const;

            /// \brief Gets the mapped IdTable.
            IdTable& getIdTable(NDGateDecl* ref);

            /// \brief Gets the quantum variable mapped to \p id from some gate.
            NodeRef getQVar(std::string id, NDGateDecl* gate = nullptr, bool recursive = true);
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
            void runPass(Pass* pass, bool force = false);

            /// \brief Clones the current qmodule.
            std::unique_ptr<QModule> clone() const;

            /// \brief Process the AST in order to obtain the QModule.
            static std::unique_ptr<QModule> GetFromAST(NodeRef ref);
            /// \brief Parses the file \p filename and returns a QModule.
            static std::unique_ptr<QModule> Parse(std::string filename, std::string path = "./",
                    bool forceStdLib = true);
            /// \brief Parses the string \p program and returns a QModule.
            static std::unique_ptr<QModule> ParseString(std::string program, bool forceStdLib = true);

            friend class QModulefyPass;

    };
}

#endif
