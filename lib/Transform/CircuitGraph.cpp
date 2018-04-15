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

Node::Ref CircuitGraph::CircuitNode::node() {
    return mNode;
}

uint32_t CircuitGraph::CircuitNode::numberOfXbits() {
    return mStepMap.size();
}

bool CircuitGraph::CircuitNode::isOutputNode() {
    return mType == CircuitNode::Type::OUTPUT;
}

bool CircuitGraph::CircuitNode::isInputNode() {
    return mType == CircuitNode::Type::INPUT;
}

bool CircuitGraph::CircuitNode::isGateNode() {
    return !isInputNode() && !isOutputNode();
}

std::vector<uint32_t> CircuitGraph::CircuitNode::getXbitsIds() {
    std::vector<uint32_t> xbitsIds;
    for (auto& pair : mStepMap) {
        xbitsIds.push_back(pair.first);
    }

    return xbitsIds;
}

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

bool CircuitGraph::Iterator::next(uint32_t id) {
    if (mPtr[id]->isOutputNode()) return false;
    mPtr[id] = mPtr[id]->mStepMap[id].second;
    return true;
}

bool CircuitGraph::Iterator::next(Xbit xbit) {
    uint32_t id = xbit.getRealId(mQubits, mCbits);
    return next(id);
}

bool CircuitGraph::Iterator::back(uint32_t id) {
    if (mPtr[id]->isInputNode()) return false;
    mPtr[id] = mPtr[id]->mStepMap[id].first;
    return true;
}

bool CircuitGraph::Iterator::back(Xbit xbit) {
    uint32_t id = xbit.getRealId(mQubits, mCbits);
    return back(id);
}

Node::Ref CircuitGraph::Iterator::get(uint32_t id) {
    return mPtr[id]->mNode;
}

Node::Ref CircuitGraph::Iterator::get(Xbit xbit) {
    uint32_t id = xbit.getRealId(mQubits, mCbits);
    return get(id);
}

CircuitGraph::CircuitNode::sRef CircuitGraph::Iterator::operator[](uint32_t id) {
    return mPtr[id];
}

CircuitGraph::CircuitNode::sRef CircuitGraph::Iterator::operator[](Xbit xbit) {
    uint32_t id = xbit.getRealId(mQubits, mCbits);
    return (*this)[id];
}

// ----------------------------------------------------------------
// -------------------- CircuitGraph Class ------------------------
// ----------------------------------------------------------------

CircuitGraph::CircuitGraph() : mInit(false) {}

CircuitGraph::CircuitGraph(uint32_t qubits, uint32_t cbits) {
    init(qubits, cbits);
}

void CircuitGraph::init(uint32_t qubits, uint32_t cbits) {
    mInit = true;
    mQubits = qubits;
    mCbits = cbits;

    mGraphHead.assign(mQubits + mCbits,
                      CircuitNode::sRef(new CircuitNode(CircuitNode::Type::INPUT)));
    mGraphTail.assign(mQubits + mCbits,
                      CircuitNode::sRef(new CircuitNode(CircuitNode::Type::OUTPUT)));

    // Initializing the head to point to the tail, and the tail to the head.
    for (uint32_t i = 0, e = mQubits + mCbits; i < e; ++i) {
        mGraphHead[i]->mStepMap[i] = std::make_pair(nullptr, mGraphTail[i]);
        mGraphTail[i]->mStepMap[i] = std::make_pair(mGraphHead[i], nullptr);
    }
}

void CircuitGraph::checkInitialized() {
    if (!mInit) {
        ERR << "Trying to append a node to an uninitialized CircuitGraph." << std::endl;
        std::exit(static_cast<uint32_t>(ExitCode::EXIT_unreachable));
    }
}

uint32_t CircuitGraph::getQSize() const {
    return mQubits;
}

uint32_t CircuitGraph::getCSize() const {
    return mCbits;
}

uint32_t CircuitGraph::size() const {
    return mQubits + mCbits;
}

void CircuitGraph::append(std::vector<Xbit> xbits, Node::Ref node) {
    checkInitialized();

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
    checkInitialized();

    Iterator iterator(mQubits, mCbits, mGraphHead);
    return iterator;
}
