#include "enfield/Transform/Pass.h"

efd::Pass::Pass(Kind k) : mK(k) {
}

efd::Pass::Kind efd::Pass::getKind() const {
    return mK;
}
