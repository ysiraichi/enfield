#include "enfield/Support/TokenSwapFinder.h"

using namespace efd;

TokenSwapFinder::TokenSwapFinder() : mG(nullptr) {}

void TokenSwapFinder::checkGraphSet() {
    if (mG == nullptr) {
        ERR << "Set the `Graph` for TokenSwapFinder." << std::endl;
        EFD_ABORT();
    }
}

void TokenSwapFinder::setGraph(Graph::Ref graph) {
    mG = graph;
    preprocess();
}

SwapSeq TokenSwapFinder::find(const InverseMap& from, const InverseMap& to) {
    checkGraphSet();
    return findImpl(from, to);
}
