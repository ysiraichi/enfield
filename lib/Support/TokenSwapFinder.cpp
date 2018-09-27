#include "enfield/Support/TokenSwapFinder.h"

using namespace efd;

TokenSwapFinder::TokenSwapFinder() : mG(nullptr) {}

void TokenSwapFinder::checkGraphSet() {
    EfdAbortIf(mG == nullptr, "Set the `Graph` for TokenSwapFinder.");
}

void TokenSwapFinder::setGraph(Graph::Ref graph) {
    mG = graph;
    preprocess();
}

SwapSeq TokenSwapFinder::find(const InverseMap& from, const InverseMap& to) {
    checkGraphSet();
    return findImpl(from, to);
}
