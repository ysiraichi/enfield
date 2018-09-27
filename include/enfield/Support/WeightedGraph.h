#ifndef __EFD_WEIGHTED_GRAPH_H__
#define __EFD_WEIGHTED_GRAPH_H__

#include "enfield/Support/Graph.h"
#include "enfield/Support/Defs.h"
#include <map>
#include <fstream>
#include <sstream>

namespace efd {
    template <typename T>
        class WeightedGraph : public Graph {
            public:
                typedef WeightedGraph<T>* Ref;
                typedef std::unique_ptr<WeightedGraph<T>> uRef;
                typedef std::shared_ptr<WeightedGraph<T>> sRef;

            private:
                std::map<std::pair<uint32_t, uint32_t>, T> mW;

            protected:
                std::string edgeToString(uint32_t i, uint32_t j, std::string op)
                    const override;

                /// \brief Constructor to be used by whoever inherits this class.
                WeightedGraph(Kind k, uint32_t n, Type ty = Undirected);

            public:
                WeightedGraph(uint32_t n, Type ty = Undirected);

                /// \brief Insert the edge (i, j) with weight w(i, j) = \p w.
                void putEdge(uint32_t i, uint32_t j, T w);

                /// \brief Sets the weight of an edge (i, j).
                void setW(uint32_t i, uint32_t j, T w);
                /// \brief Gets the weight of an edge (i, j).
                T getW(uint32_t i, uint32_t j) const;
        
                /// \brief Returns true if \p g is of this type.
                static bool ClassOf(const Graph* g);
        
                /// \brief Encapsulates the creation of a new Graph.
                static uRef Create(uint32_t n, Type ty = Undirected);
        };

    template <class T> struct JsonFields<WeightedGraph<T>> {
        static std::string _WeightLabel_;
    };

    template <class T> struct JsonBackendParser<WeightedGraph<T>> {
        static T ParseWeight(const Json::Value& v);
        static std::vector<Json::ValueType> GetTysForT();
        static std::unique_ptr<WeightedGraph<T>> Parse(const Json::Value& root);
    };

    template <> int32_t JsonBackendParser<WeightedGraph<int32_t>>::
        ParseWeight(const Json::Value& v);
    template <> uint32_t JsonBackendParser<WeightedGraph<uint32_t>>::
        ParseWeight(const Json::Value& v);
    template <> double JsonBackendParser<WeightedGraph<double>>::
        ParseWeight(const Json::Value& v);

    template <> std::vector<Json::ValueType>
        JsonBackendParser<WeightedGraph<int32_t>>::GetTysForT();
    template <> std::vector<Json::ValueType>
        JsonBackendParser<WeightedGraph<uint32_t>>::GetTysForT();
    template <> std::vector<Json::ValueType>
        JsonBackendParser<WeightedGraph<double>>::GetTysForT();
}

template <typename T>
efd::WeightedGraph<T>::WeightedGraph(Kind k, uint32_t n, Type ty) : Graph(k, n, ty) {}

template <typename T>
efd::WeightedGraph<T>::WeightedGraph(uint32_t n, Type ty) : Graph(K_WEIGHTED, n, ty) {}

template <typename T>
std::string efd::WeightedGraph<T>::edgeToString(uint32_t i, uint32_t j,
                                                std::string op) const {
    return vertexToString(i) + " " + op + " " + vertexToString(j) +
        "[label=" + std::to_string(getW(i, j)) + "]";
}

template <typename T>
void efd::WeightedGraph<T>::putEdge(uint32_t i, uint32_t j, T w) {
    Graph::putEdge(i, j);

    mW[std::make_pair(i, j)] = w;
    if (!isDirectedGraph()) {
        mW[std::make_pair(j, i)] = w;
    }
}

template <typename T>
void efd::WeightedGraph<T>::setW(uint32_t i, uint32_t j, T w) {
    auto pair = std::make_pair(i, j);
    EfdAbortIf(mW.find(pair) == mW.end(), "Edge not found: `(" << i << ", " << j << ")`.");
    mW[pair] = w;
}

template <typename T>
T efd::WeightedGraph<T>::getW(uint32_t i, uint32_t j) const {
    auto pair = std::make_pair(i, j);

    EfdAbortIf(mW.find(pair) == mW.end(),
               "Edge weight not found for edge: `(" << i << ", " << j << ")`.");

    return mW.at(pair);
}

template <typename T>
bool efd::WeightedGraph<T>::ClassOf(const Graph* g) {
    return g->isWeighted();
}

template <typename T>
typename efd::WeightedGraph<T>::uRef efd::WeightedGraph<T>::Create(uint32_t n, Type ty) {
    return std::unique_ptr<WeightedGraph<T>>(new WeightedGraph<T>(n, ty));
}

// ----------------------------- JsonFields -------------------------------
template <class T>
std::string efd::JsonFields<efd::WeightedGraph<T>>::_WeightLabel_ = "w";

// ----------------------------- JsonBackendParser -------------------------------
template <class T>
T efd::JsonBackendParser<efd::WeightedGraph<T>>::ParseWeight(const Json::Value& v) {
    EfdAbortIf(true, "ParseWeight not implemented for `" << typeid(T).name() << "`.");
}

template <class T>
std::vector<Json::ValueType>
efd::JsonBackendParser<efd::WeightedGraph<T>>::GetTysForT() {
    EfdAbortIf(true, "ParseWeight not implemented for `" << typeid(T).name() << "`.");
}

template <class T>
std::unique_ptr<efd::WeightedGraph<T>>
efd::JsonBackendParser<efd::WeightedGraph<T>>::Parse(const Json::Value& root) {
    typedef Json::ValueType Ty;
    const std::string _WGraphErrorPrefix_ = "WeightedGraph parsing error";

    JsonCheckTypeError(_WGraphErrorPrefix_, root, JsonFields<Graph>::_VerticesLabel_,
                       { Ty::intValue, Ty::uintValue });
    JsonCheckTypeError(_WGraphErrorPrefix_, root, JsonFields<Graph>::_AdjListLabel_,
                       { Ty::arrayValue });
    JsonCheckTypeError(_WGraphErrorPrefix_, root, JsonFields<Graph>::_TypeLabel_,
                       { Ty::stringValue });

    auto vertices = root[JsonFields<Graph>::_VerticesLabel_].asUInt();
    auto typeStr = root[JsonFields<Graph>::_TypeLabel_].asString();
    auto ty = Graph::Type::Undirected;

    if (typeStr == "Directed") {
        ty = Graph::Type::Directed;
    } else if (typeStr != "Undirected") {
        WAR << "Graph parsing warning: defaulting to `Undirected type`." << std::endl;
    }

    auto graph = WeightedGraph<T>::Create(vertices, ty);
    auto &adj = root[JsonFields<Graph>::_AdjListLabel_];

    for (uint32_t i = 0; i < vertices; ++i) {
        JsonCheckTypeError(_WGraphErrorPrefix_, adj, i, { Ty::arrayValue });
        auto &iList = adj[i];
        
        for (uint32_t j = 0, f = iList.size(); j < f; ++j) {
            JsonCheckTypeError(_WGraphErrorPrefix_, iList, j, { Ty::objectValue });
            auto &jElem = iList[j];
            JsonCheckTypeError(_WGraphErrorPrefix_, jElem, JsonFields<Graph>::_VLabel_,
                               { Ty::intValue, Ty::uintValue });
            JsonCheckTypeError(_WGraphErrorPrefix_, jElem,
                               JsonFields<WeightedGraph<T>>::_WeightLabel_,
                               JsonBackendParser<WeightedGraph<T>>::GetTysForT());
            auto v = jElem[JsonFields<Graph>::_VLabel_].asUInt();
            auto w = JsonBackendParser<WeightedGraph<T>>::ParseWeight
                (jElem[JsonFields<WeightedGraph<T>>::_WeightLabel_]);
            graph->putEdge(i, v, w);
        }
    }

    return graph;
}

#endif
