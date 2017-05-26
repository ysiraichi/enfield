#ifndef __EFD_ID_TABLE_H__
#define __EFD_ID_TABLE_H__

#include "enfield/Analysis/Nodes.h"

#include <string>
#include <unordered_map>

namespace efd {

    /// \brief Recursive table mapping ids to nodes.
    class IdTable {
        private:
            struct Record {
                std::string mId;
                NodeRef mNode;
                bool mIsGate;
            };
            
            std::unordered_map<std::string, Record> mMap;
            IdTable* mParent;

        public:
            IdTable(IdTable* parent = nullptr);

            /// \brief Adds a quantum variable mapping from \p id to \p node.
            void addQVar(std::string id, NodeRef node);
            /// \brief Adds a quantum gate mapping from \p id to \p node.
            void addQGate(std::string id, NodeRef node);

            /// \brief Gets the quantum variable mapped to \p id.
            NodeRef getQVar(std::string id, bool recursive = true);
            /// \brief Gets the quantum gate mapped to \p id.
            NDGateDecl* getQGate(std::string id, bool recursive = true);

            /// \brief Returns the parent table.
            IdTable* getParent();

            /// \brief Clears the table.
            void clear();

            static IdTable* Create(IdTable* parent = nullptr);
    };
};

#endif
