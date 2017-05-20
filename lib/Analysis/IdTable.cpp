#include "enfield/Analysis/IdTable.h"
#include "enfield/Support/RTTI.h"

efd::IdTable::IdTable(IdTable* parent) : mParent(parent) {
}

void efd::IdTable::addQVar(std::string id, NodeRef node) {
    mMap[id] = Record { id, node, false };
}

void efd::IdTable::addQGate(std::string id, NodeRef node) {
    mMap[id] = Record { id, node, true };
}

efd::NodeRef efd::IdTable::getQVar(std::string id, bool recursive) {
    if (mMap.find(id) != mMap.end()) {
        Record record = mMap[id];
        if (!record.mIsGate)
            return record.mNode;
    }

    if (recursive && mParent != nullptr) {
        return mParent->getQVar(id);
    }

    return nullptr;
}

efd::NDGateDecl* efd::IdTable::getQGate(std::string id, bool recursive) {
    if (mMap.find(id) != mMap.end()) {
        Record record = mMap[id];
        if (record.mIsGate)
            return dynCast<NDGateDecl>(record.mNode);
    }

    if (recursive && mParent != nullptr) {
        return mParent->getQGate(id);
    }

    return nullptr;
}

efd::IdTable* efd::IdTable::getParent() {
    return mParent;
}

efd::IdTable* efd::IdTable::create(IdTable* parent) {
    return new IdTable(parent);
}
