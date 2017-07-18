#ifndef __EFD_ID_TABLE_H__
#define __EFD_ID_TABLE_H__

#include "enfield/Analysis/Nodes.h"

#include <string>
#include <unordered_map>

namespace efd {

    /// \brief Recursive table mapping ids to nodes.
    class IdTable {
        public:
            typedef IdTable* Ref;
            typedef std::unique_ptr<IdTable> uRef;
            typedef std::shared_ptr<IdTable> sRef;

        private:
            struct Record {
                std::string mId;
                Node::Ref mNode;
                bool mIsGate;
            };
            
            std::unordered_map<std::string, Record> mMap;
            Ref mParent;

        public:
            IdTable(Ref parent = nullptr);

            /// \brief Adds a quantum variable mapping from \p id to \p node.
            void addQVar(std::string id, Node::Ref node);
            /// \brief Adds a quantum gate mapping from \p id to \p node.
            void addQGate(std::string id, Node::Ref node);

            /// \brief Gets the quantum variable mapped to \p id.
            Node::Ref getQVar(std::string id, bool recursive = true);
            /// \brief Gets the quantum gate mapped to \p id.
            NDGateDecl::Ref getQGate(std::string id, bool recursive = true);

            /// \brief Returns the parent table.
            Ref getParent();

            /// \brief Clears the table.
            void clear();

            static uRef Create(Ref parent = nullptr);
    };
};

#endif
