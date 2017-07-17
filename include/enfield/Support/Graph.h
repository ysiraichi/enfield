#ifndef __EFD_GRAPH_H__
#define __EFD_GRAPH_H__

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

        protected:
            unsigned mN;
            std::vector<std::set<unsigned>> mSuccessors;
            std::vector<std::set<unsigned>> mPredecessors;

            Graph(unsigned n);

        public:
            /// \brief Return the degree entering the vertex \p i.
            unsigned inDegree(unsigned i) const;
            /// \brief Return the degree leaving the vertex \p i.
            unsigned outDegree(unsigned i) const; 
            /// \brief Return the number of vertices.
            unsigned size() const;
    
            /// \brief Return the set of succesors of some vertex \p i.
            std::set<unsigned>& succ(unsigned i);
            /// \brief Return the set of predecessors of some vertex \p i.
            std::set<unsigned>& pred(unsigned i); 
    
            /// \brief Inserts an edge (i, j) in the successor's list and
            /// an edge (j, i) in the predecessor's list.
            void putEdge(unsigned i, unsigned j);
    
            /// \brief Returns true whether it has an edge (i, j).
            bool hasEdge(unsigned i, unsigned j); 
    
            /// \brief Encapsulates the creation of a new Graph.
            static uRef Create(unsigned n);
            /// \brief Parses the file \p filename into a Graph representation.
            static uRef Read(std::string filepath);
            /// \brief Parses the string \p graphStr into a Graph representation.
            static uRef ReadString(std::string graphStr);
    };
}

#endif
