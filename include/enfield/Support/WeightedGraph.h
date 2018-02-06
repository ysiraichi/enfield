#ifndef __EFD_WEIGHTED_GRAPH_H__
#define __EFD_WEIGHTED_GRAPH_H__

#include "enfield/Support/Graph.h"
#include "enfield/Support/Defs.h"
#include <map>
#include <fstream>
#include <sstream>
#include <cassert>

namespace efd {

    template <typename T>
    class WeightedGraph : public Graph {
        public:
            typedef WeightedGraph<T>* Ref;
            typedef std::unique_ptr<WeightedGraph<T>> uRef;
            typedef std::shared_ptr<WeightedGraph<T>> sRef;

        protected:
            std::string edgeToString(uint32_t i, uint32_t j, std::string op)
                const override;

        private:
            std::map<std::pair<uint32_t, uint32_t>, T> mW;
            static std::unique_ptr<WeightedGraph<T>> ReadFromIn(std::istream& in,
                                                                Type ty = Undirected);

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
            /// \brief Parses the file \p filename into a Graph representation.
            static uRef Read(std::string filepath, Type ty = Undirected);
            /// \brief Parses the string \p graphStr into a Graph representation.
            static uRef ReadString(std::string graphStr, Type ty = Undirected);
    };
}

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
    assert(mW.find(pair) != mW.end() && "Edge weight not found.");
    mW[pair] = w;
}

template <typename T>
T efd::WeightedGraph<T>::getW(uint32_t i, uint32_t j) const {
    auto pair = std::make_pair(i, j);
    assert(mW.find(pair) != mW.end() && "Edge weight not found.");
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

template <typename T> typename efd::WeightedGraph<T>::uRef
efd::WeightedGraph<T>::ReadFromIn(std::istream& in, Type ty) {
    uint32_t n;
    in >> n;

    std::unique_ptr<efd::WeightedGraph<T>> graph(efd::WeightedGraph<T>::Create(n, ty));

    T w;
    for (uint32_t u, v; in >> u >> v >> w;)
        graph->putEdge(u, v, w);

    return graph;
}

template <typename T> typename efd::WeightedGraph<T>::uRef
efd::WeightedGraph<T>::Read(std::string filepath, Type ty) {
    std::ifstream in(filepath.c_str());
    return ReadFromIn(in, ty);
}

template <typename T> typename efd::WeightedGraph<T>::uRef
efd::WeightedGraph<T>::ReadString(std::string graphStr, Type ty) {
    std::stringstream in(graphStr);
    return ReadFromIn(in, ty);
}


#endif
