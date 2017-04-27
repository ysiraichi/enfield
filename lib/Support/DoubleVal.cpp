#include "enfield/Support/DoubleVal.h"

efd::DoubleVal::DoubleVal() : mV(0), mStr("") {
}

efd::DoubleVal::DoubleVal(std::string str) : mStr(str) {
    mV = std::stod(mStr);
}
