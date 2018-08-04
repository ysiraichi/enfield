#include "enfield/Support/TokenSwapFinder.h"

using namespace efd;

TokenSwapFinder::TokenSwapFinder() : mG(nullptr) {}

void TokenSwapFinder::checkGraphSet() {
    if (mG == nullptr) {
        ERR << "Set the `Graph` for TokenSwapFinder." << std::endl;
        ExitWith(ExitCode::EXIT_unreachable);
    }
}

void TokenSwapFinder::setGraph(Graph::Ref graph) {
    mG = graph;
    preprocess();
}

SwapSeq TokenSwapFinder::find(const Assign& from, const Assign& to) {
    checkGraphSet();
    return findImpl(from, to);
}
