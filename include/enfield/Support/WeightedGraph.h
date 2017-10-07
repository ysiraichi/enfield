#ifndef __EFD_WEIGHTED_GRAPH_H__
#define __EFD_WEIGHTED_GRAPH_H__

#include "enfield/Support/Graph.h"
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

            WeightedGraph(uint32_t n);
            static std::unique_ptr<WeightedGraph<T>> ReadFromIn(std::istream& in);

        public:
            /// \brief Insert the edge (i, j) with weight w(i, j) = \p w.
            void putEdge(uint32_t i, uint32_t j, T w);

            /// \brief Gets the weight of an edge (i, j).
            T getW(uint32_t i, uint32_t j);
    
            /// \brief Returns true if \p g is of this type.
            static bool ClassOf(const Graph* g);
    
            /// \brief Encapsulates the creation of a new Graph.
            static uRef Create(uint32_t n);
            /// \brief Parses the file \p filename into a Graph representation.
            static uRef Read(std::string filepath);
            /// \brief Parses the string \p graphStr into a Graph representation.
            static uRef ReadString(std::string graphStr);
    };
}

template <typename T>
efd::WeightedGraph<T>::WeightedGraph(uint32_t n) : Graph(K_WEIGHTED, n) {}

template <typename T>
void efd::WeightedGraph<T>::putEdge(uint32_t i, uint32_t j, T w) {
    Graph::putEdge(i, j);
    mW[std::make_pair(i, j)] = w;
}

template <typename T>
T efd::WeightedGraph<T>::getW(uint32_t i, uint32_t j) {
    auto pair = std::make_pair(i, j);
    assert(mW.find(pair) != mW.end() && "Edge weight not found.");
    return mW[pair];
}

template <typename T>
bool efd::WeightedGraph<T>::ClassOf(const Graph* g) {
    return g->isWeighted();
}

template <typename T>
typename efd::WeightedGraph<T>::uRef efd::WeightedGraph<T>::Create(uint32_t n) {
    return std::unique_ptr<WeightedGraph<T>>(new WeightedGraph<T>(n));
}

template <typename T>
typename efd::WeightedGraph<T>::uRef efd::WeightedGraph<T>::ReadFromIn(std::istream& in) {
    uint32_t n;
    in >> n;

    std::unique_ptr<efd::WeightedGraph<T>> graph(efd::WeightedGraph<T>::Create(n));

    T w;
    for (uint32_t u, v; in >> u >> v >> w;)
        graph->putEdge(u, v, w);

    return graph;
}

template <typename T>
typename efd::WeightedGraph<T>::uRef efd::WeightedGraph<T>::Read(std::string filepath) {
    std::ifstream in(filepath.c_str());
    return ReadFromIn(in);
}

template <typename T>
typename efd::WeightedGraph<T>::uRef efd::WeightedGraph<T>::ReadString(std::string graphStr) {
    std::stringstream in(graphStr);
    return ReadFromIn(in);
}


#endif
