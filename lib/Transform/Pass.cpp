#include "enfield/Transform/Pass.h"

efd::Pass::Pass(Kind k) : mK(k) {
}

efd::Pass::Kind efd::Pass::getKind() const {
    return mK;
}

efd::PassT<void>::PassT() : Pass(K_VOID) {
}

bool efd::PassT<void>::ClassOf(Pass* ref) {
    return ref->getKind() == K_VOID;
}
