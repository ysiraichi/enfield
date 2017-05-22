#include "enfield/Support/Graph.h"

#include <iostream>

efd::Graph::Graph(unsigned n) : mN(n) {
    mSuccessors.assign(n, std::set<unsigned>());
    mPredecessors.assign(n, std::set<unsigned>());
}

unsigned efd::Graph::inDegree(unsigned i) const {
    return mPredecessors[i].size();
}

unsigned efd::Graph::outDegree(unsigned i) const {
    return mSuccessors[i].size();
}

unsigned efd::Graph::size() const {
    return mN;
}

std::set<unsigned>& efd::Graph::succ(unsigned i) {
    return mSuccessors[i];
}

std::set<unsigned>& efd::Graph::pred(unsigned i) {
    return mPredecessors[i];
}

bool efd::Graph::hasEdge(unsigned i, unsigned j) {
    std::set<unsigned>& succ = this->succ(i);
    return succ.find(j) != succ.end();
}

void efd::Graph::putEdge(unsigned i, unsigned j) {
    std::set<unsigned>& succ = this->succ(i);
    std::set<unsigned>& pred = this->pred(j);

    succ.insert(j);
    pred.insert(i);
}

bool efd::Graph::isReverseEdge(unsigned i, unsigned j) {
    return mReverseEdges.find(std::pair<unsigned, unsigned>(i, j)) != mReverseEdges.end();
}

void efd::Graph::buildReverseGraph() {
    for (unsigned i = 0; i < mN; ++i) {
        std::set<unsigned>& succ = this->succ(i);

        for (unsigned k : succ) {
            if (!hasEdge(k, i)) {
                putEdge(k, i);
                mReverseEdges.insert(std::pair<unsigned, unsigned>(k, i));
            }
        }
    }
}

void efd::Graph::print() {
    for (unsigned i = 0; i < mN; ++i) {
        const std::set<unsigned> &succ = mSuccessors[i];
        const std::set<unsigned> &pred = mPredecessors[i];

        std::cout << i << " -> ";
        for (unsigned k : succ)
            std::cout << "(" << k << ") ";
        std::cout << std::endl;

        std::cout << i << " <- ";
        for (unsigned k : pred)
            std::cout << "(" << k << ") ";
        std::cout << std::endl;
    }
}

