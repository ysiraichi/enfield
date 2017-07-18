#include "enfield/Transform/IdTable.h"
#include "enfield/Support/RTTI.h"

efd::IdTable::IdTable(Ref parent) : mParent(parent) {
}

void efd::IdTable::addQVar(std::string id, Node::Ref node) {
    mMap[id] = Record { id, node, false };
}

void efd::IdTable::addQGate(std::string id, Node::Ref node) {
    mMap[id] = Record { id, node, true };
}

efd::Node::Ref efd::IdTable::getQVar(std::string id, bool recursive) {
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

efd::IdTable::Ref efd::IdTable::getParent() {
    return mParent;
}

void efd::IdTable::clear() {
    mMap.clear();
    mParent = nullptr;
}

efd::IdTable::uRef efd::IdTable::Create(Ref parent) {
    return uRef(new IdTable(parent));
}
