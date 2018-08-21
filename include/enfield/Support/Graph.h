#ifndef __EFD_GRAPH_H__
#define __EFD_GRAPH_H__

#include "enfield/Support/JsonParser.h"

#include <set>
#include <vector>
#include <unordered_map>
#include <memory>

namespace efd {
    /// \brief Graph representation.
    class Graph {
        public:
            typedef Graph* Ref;
            typedef std::unique_ptr<Graph> uRef;
            typedef std::shared_ptr<Graph> sRef;

            enum Kind {
                K_GRAPH,
                K_WEIGHTED,
                K_ARCH
            };

            enum Type { Directed, Undirected };

        protected:
            Kind mK;
            uint32_t mN;
            Type mTy;

            std::vector<std::set<uint32_t>> mSuccessors;
            std::vector<std::set<uint32_t>> mPredecessors;

            Graph(Kind k, uint32_t n, Type ty = Undirected);

            virtual std::string vertexToString(uint32_t i) const;
            virtual std::string edgeToString(uint32_t i, uint32_t j, std::string op) const;

        public:
            virtual ~Graph() = default;
            Graph(uint32_t n, Type ty = Undirected);

            /// \brief Return the degree entering the vertex \p i.
            uint32_t inDegree(uint32_t i) const;
            /// \brief Return the degree leaving the vertex \p i.
            uint32_t outDegree(uint32_t i) const; 
            /// \brief Return the number of vertices.
            uint32_t size() const;
    
            /// \brief Return the set of succesors of some vertex \p i.
            std::set<uint32_t>& succ(uint32_t i);
            /// \brief Return the set of predecessors of some vertex \p i.
            std::set<uint32_t>& pred(uint32_t i); 
            /// \brief Return the set of adjacent vertices of some vertex \p i.
            std::set<uint32_t> adj(uint32_t i) const;
    
            /// \brief Inserts an edge (i, j) in the successor's list and
            /// an edge (j, i) in the predecessor's list.
            void putEdge(uint32_t i, uint32_t j);
            /// \brief Returns true whether it has an edge (i, j).
            bool hasEdge(uint32_t i, uint32_t j); 

            /// \brief Returns true if this is a weighted graph.
            bool isWeighted() const;
            /// \brief Returns true if this is an architecture graph.
            bool isArch() const;

            /// \brief Returns true if this is a directed graph.
            bool isDirectedGraph() const;

            /// \brief Converts itself to a 'dot' graph representation.
            std::string dotify(std::string name = "Dump") const;

            /// \brief Returns true if \p g is of this type.
            static bool ClassOf(const Graph* g);
    
            /// \brief Encapsulates the creation of a new Graph.
            static uRef Create(uint32_t n, Type ty = Undirected);
    };

    template <> struct JsonFields<Graph> {
        static const std::string _VerticesLabel_;
        static const std::string _AdjListLabel_;
        static const std::string _TypeLabel_;
        static const std::string _VLabel_;
    };

    template <> struct JsonBackendParser<Graph> {
        static std::unique_ptr<Graph> Parse(const Json::Value& root);
    };
}

#endif
