#include "enfield/Transform/CircuitGraph.h"
#include "enfield/Support/Defs.h"

#include <cassert>

using namespace efd;

// ----------------------------------------------------------------
// -------------------------- Xbit Class --------------------------
// ----------------------------------------------------------------

Xbit::Xbit(Type t, uint32_t id) : mType(t), mId(id) {}

uint32_t Xbit::getRealId(uint32_t qubits, uint32_t cbits) {
    uint32_t id = mId;

    switch (mType) {
        case Type::QUANTUM:
            if (id >= qubits) {
                ERR <<  "Qubit with id `" << id << "`, but max `"
                    << qubits - 1 << "`." << std::endl;
                std::exit(static_cast<uint32_t>(ExitCode::EXIT_unreachable));
            }
            break;

        case Type::CLASSIC:
            id = id + qubits;
            if (id >= qubits + cbits) {
                ERR <<  "Classical bit with id `" << id << "`, but max `"
                    << qubits + cbits - 1 << "`." << std::endl;
                std::exit(static_cast<uint32_t>(ExitCode::EXIT_unreachable));
            }
            break;
    }

    return id;
}

Xbit Xbit::Q(uint32_t id) {
    return Xbit(Type::QUANTUM, id);
}

Xbit Xbit::C(uint32_t id) {
    return Xbit(Type::CLASSIC, id);
}

// ----------------------------------------------------------------
// --------------------- CircuitNode Class ------------------------
// ----------------------------------------------------------------

CircuitGraph::CircuitNode::CircuitNode(Type type) : mType(type), mNode(nullptr) {}

// ----------------------------------------------------------------
// ----------------- CircuitGraph::Iterator Class -----------------
// ----------------------------------------------------------------

CircuitGraph::Iterator::Iterator(uint32_t qubits,
                                 uint32_t cbits,
                                 std::vector<CircuitNode::sRef> ptr) :
                                 mQubits(qubits),
                                 mCbits(cbits),
                                 mPtr(ptr) {}

CircuitGraph::Iterator::Iterator() : mQubits(0), mCbits(0), mPtr(0) {}

bool CircuitGraph::Iterator::next(Xbit xbit) {
    if (endOfCircuit(xbit)) return false;
    uint32_t id = xbit.getRealId(mQubits, mCbits);
    mPtr[id] = mPtr[id]->mStepMap[id].second;
    return true;
}

bool CircuitGraph::Iterator::back(Xbit xbit) {
    if (beginningOfCircuit(xbit)) return false;
    uint32_t id = xbit.getRealId(mQubits, mCbits);
    mPtr[id] = mPtr[id]->mStepMap[id].first;
    return true;
}

Node::Ref CircuitGraph::Iterator::get(Xbit xbit) {
    if (endOfCircuit(xbit)) return nullptr;
    uint32_t id = xbit.getRealId(mQubits, mCbits);
    return mPtr[id]->mNode;
}

bool CircuitGraph::Iterator::endOfCircuit() {
    for (uint32_t i = 0, e = mQubits + mCbits; i < e; ++i) {
        if (mPtr[i]->mType != CircuitNode::Type::OUTPUT) {
            return false;
        }
    }

    return true;
}

bool CircuitGraph::Iterator::endOfCircuit(Xbit xbit) {
    uint32_t id = xbit.getRealId(mQubits, mCbits);
    assert(id < mPtr.size() && "Invalid qubit request.");
    return mPtr[id]->mType == CircuitNode::Type::OUTPUT;
}

bool CircuitGraph::Iterator::beginningOfCircuit(Xbit xbit) {
    uint32_t id = xbit.getRealId(mQubits, mCbits);
    assert(id < mPtr.size() && "Invalid qubit request.");
    assert(mPtr[id] && "Invalid qubit request.");
    return mPtr[id]->mType == CircuitNode::Type::INPUT;
}

// ----------------------------------------------------------------
// -------------------- CircuitGraph Class ------------------------
// ----------------------------------------------------------------

CircuitGraph::CircuitGraph(uint32_t qubits, uint32_t cbits) :
    mQubits(qubits),
    mCbits(cbits),
    mGraphHead(mQubits + mCbits, CircuitNode::sRef(new CircuitNode(CircuitNode::Type::INPUT))),
    mGraphTail(mQubits + mCbits, CircuitNode::sRef(new CircuitNode(CircuitNode::Type::OUTPUT))) {

    // Initializing the head to point to the tail, and the tail to the head.
    for (uint32_t i = 0, e = mQubits + mCbits; i < e; ++i) {
        mGraphHead[i]->mStepMap[i] = std::make_pair(nullptr, mGraphTail[i]);
        mGraphTail[i]->mStepMap[i] = std::make_pair(mGraphHead[i], nullptr);
    }
}

void CircuitGraph::append(std::vector<Xbit> xbits, Node::Ref node) {
    CircuitNode::sRef newNode(new CircuitNode(CircuitNode::Type::GATE));
    newNode->mNode = node;

    for (auto xbit : xbits) {
        uint32_t id = xbit.getRealId(mQubits, mCbits);

        auto lastNode = mGraphTail[id]->mStepMap[id].first;
        lastNode->mStepMap[id].second = newNode;
        newNode->mStepMap[id] = std::make_pair(lastNode, mGraphTail[id]);
        mGraphTail[id]->mStepMap[id].first = newNode;
    }
}

CircuitGraph::Iterator CircuitGraph::build_iterator() {
    Iterator iterator(mQubits, mCbits, mGraphHead);
    return iterator;
}
