#include "enfield/Support/WeightedGraph.h"

using namespace efd;

template <>
int32_t JsonBackendParser<WeightedGraph<int32_t>>::ParseWeight(const Json::Value& v) {
    return v.asInt();
}

template <>
std::vector<Json::ValueType> JsonBackendParser<WeightedGraph<int32_t>>::GetTysForT() {
    return { Json::ValueType::intValue };
}

template <>
uint32_t JsonBackendParser<WeightedGraph<uint32_t>>::ParseWeight(const Json::Value& v) {
    return v.asUInt();
}

template <>
std::vector<Json::ValueType> JsonBackendParser<WeightedGraph<uint32_t>>::GetTysForT() {
    return { Json::ValueType::intValue, Json::ValueType::uintValue };
}

template <>
double JsonBackendParser<WeightedGraph<double>>::ParseWeight(const Json::Value& v) {
    return v.asDouble();
}

template <>
std::vector<Json::ValueType> JsonBackendParser<WeightedGraph<double>>::GetTysForT() {
    return {  Json::ValueType::intValue,
              Json::ValueType::uintValue,
              Json::ValueType::realValue };
}
