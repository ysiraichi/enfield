#ifndef __EFD_QMODULE_H__
#define __EFD_QMODULE_H__

#include "enfield/Analysis/Nodes.h"

#include <unordered_map>

namespace efd {
    class QModulefyPass;

    /// \brief Qasm module representation.
    class QModule {
        private:
            NodeRef mAST;
            NodeRef mVersion;

            std::vector<NodeRef> mRegDecls;
            std::vector<NodeRef> mGates;
            std::vector<NodeRef> mStatements;

            QModule();

        public:
            typedef std::vector<NodeRef>::iterator Iterator;
            typedef std::vector<NodeRef>::const_iterator ConstIterator;

            /// \brief Gets the qasm version.
            NodeRef getVersion();

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

            /// \brief Process the AST in order to obtain the QModule.
            static std::unique_ptr<QModule> GetFromAST(NodeRef ref);
            /// \brief Parses the file \p filename and returns a QModule.
            static std::unique_ptr<QModule> Parse(std::string filename, std::string path = "./");


            friend class QModulefyPass;

    };
}

#endif
