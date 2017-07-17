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
            typedef std::shared_ptr<WeightedGraph<T>> uRef;

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
            static uRef Create(unsigned n);
            /// \brief Parses the file \p filename into a Graph representation.
            static uRef Read(std::string filepath);
            /// \brief Parses the string \p graphStr into a Graph representation.
            static uRef ReadString(std::string graphStr);
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
typename efd::WeightedGraph<T>::uRef efd::WeightedGraph<T>::Create(unsigned n) {
    return std::unique_ptr<WeightedGraph<T>>(new WeightedGraph<T>(n));
}

template <typename T>
typename efd::WeightedGraph<T>::uRef efd::WeightedGraph<T>::ReadFromIn(std::istream& in) {
    unsigned n;
    in >> n;

    std::unique_ptr<efd::WeightedGraph<T>> graph(efd::WeightedGraph<T>::Create(n));

    T w;
    for (unsigned u, v; in >> u >> v >> w;)
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
