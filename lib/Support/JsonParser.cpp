#include "enfield/Support/JsonParser.h"
#include "enfield/Support/Defs.h"

using namespace efd;

std::string efd::JsonTypeString(const Json::ValueType& ty) {
    switch (ty) {
        case Json::ValueType::nullValue:    return "null";
        case Json::ValueType::intValue:     return "int";
        case Json::ValueType::uintValue:    return "uint";
        case Json::ValueType::realValue:    return "real";
        case Json::ValueType::stringValue:  return "string";
        case Json::ValueType::booleanValue: return "boolean";
        case Json::ValueType::arrayValue:   return "array";
        case Json::ValueType::objectValue:  return "object";
    }

    EfdAbortIf(true, "Bad JsonCpp ValueType.");
}

std::string efd::JsonTypeVectorString(const std::vector<Json::ValueType>& tys) {
    std::string str = "`" + JsonTypeString(tys[0]) + "`";

    for (uint32_t i = 1, e = tys.size(); i < e; ++i) {
        str += " or `" + JsonTypeString(tys[i]) + "`";
    }

    return str;
}
