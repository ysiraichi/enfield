#include "enfield/Support/WrapperVal.h"

template <>
efd::WrapperVal<long long>::WrapperVal(std::string str) : mStr(str) {
    mV = std::stoll(str);
}

template <>
efd::WrapperVal<double>::WrapperVal(std::string str) : mStr(str) {
    mV = std::stod(str);
}
