#ifndef __EFD_ID_TABLE_H__
#define __EFD_ID_TABLE_H__

#include <string>
#include <unordered_map>

namespace efd {
    class NodeRef;

    class IdTable {
        private:
            struct Record {
                std::string mId;
                NodeRef* mNode;
                bool mIsGate;
            };
            
            std::unordered_map<std::string, Record> mMap;
            IdTable* parent;

        public:
            IdTable(IdTable* parent = nullptr);

            void addQVar(NodeRef* node);
            void addQGate(NodeRef* node);

            NodeRef getQVar(std::string id);
            NodeRef getQGate(std::string id);
    };
};

#endif
