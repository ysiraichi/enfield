#ifndef __EFD_WEIGHTED_GRAPH_H__
#define __EFD_WEIGHTED_GRAPH_H__

#include "enfield/Support/Graph.h"
#include <map>
#include <fstream>
#include <sstream>

namespace efd {

    template <typename T>
    class WeightedGraph : public Graph {
        private:
            std::map<std::pair<unsigned, unsigned>, T> mW;

            WeightedGraph(unsigned n);
            static std::unique_ptr<WeightedGraph<T>> ReadFromIn(std::istream& in);

        public:
            /// \brief Insert the edge (i, j) with weight w(i, j) = \p w.
            void putEdge(unsigned i, unsigned j, T w);

            /// \brief Gets the weight of an edge (i, j).
            T getW(unsigned i, unsigned j);
    
            /// \brief Encapsulates the creation of a new Graph.
            static std::unique_ptr<WeightedGraph<T>> Create(unsigned n);
            /// \brief Parses the file \p filename into a Graph representation.
            static std::unique_ptr<WeightedGraph<T>> Read(std::string filepath);
            /// \brief Parses the string \p graphStr into a Graph representation.
            static std::unique_ptr<WeightedGraph<T>> ReadString(std::string graphStr);
    };
}

template <typename T>
efd::WeightedGraph<T>::WeightedGraph(unsigned n) : Graph(n) {}

template <typename T>
void efd::WeightedGraph<T>::putEdge(unsigned i, unsigned j, T w) {
    Graph::putEdge(i, j);
    mW[std::make_pair(i, j)] = w;
}

template <typename T>
T efd::WeightedGraph<T>::getW(unsigned i, unsigned j) {
    auto pair = std::make_pair(i, j);
    assert(mW.find(pair) != mW.end() && "Edge weight not found.");
    return mW[pair];
}

template <typename T>
std::unique_ptr<efd::WeightedGraph<T>> efd::WeightedGraph<T>::Create(unsigned n) {
    return std::unique_ptr<WeightedGraph<T>>(new WeightedGraph<T>(n));
}

template <typename T>
std::unique_ptr<efd::WeightedGraph<T>> efd::WeightedGraph<T>::ReadFromIn(std::istream& in) {
    unsigned n;
    in >> n;

    std::unique_ptr<efd::WeightedGraph<T>> graph(efd::WeightedGraph<T>::Create(n));

    T w;
    for (unsigned u, v; in >> u >> v >> w;)
        graph->putEdge(u, v, w);

    return graph;
}

template <typename T>
std::unique_ptr<efd::WeightedGraph<T>> efd::WeightedGraph<T>::Read(std::string filepath) {
    std::ifstream in(filepath.c_str());
    return ReadFromIn(in);
}

template <typename T>
std::unique_ptr<efd::WeightedGraph<T>> efd::WeightedGraph<T>::ReadString(std::string graphStr) {
    std::stringstream in(graphStr);
    return ReadFromIn(in);
}


#endif
