#ifndef __EFD_GRAPH_H__
#define __EFD_GRAPH_H__

#include <set>
#include <vector>
#include <unordered_map>
#include <memory>

namespace efd {
    /// \brief Graph representation.
    class Graph {
        private:
            unsigned mN;
            std::vector<std::set<unsigned>> mSuccessors;
            std::vector<std::set<unsigned>> mPredecessors;
            std::set<std::pair<unsigned, unsigned>> mReverseEdges;

            std::vector<std::string> mId; 
            std::unordered_map<std::string, unsigned> mStrToId;
    
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
            /// \brief Inserts a vertex called \p s and return its new unsigned id.
            unsigned putVertex(std::string s);
    
            /// \brief Returns true whether it has an edge (i, j).
            bool hasEdge(unsigned i, unsigned j); 
            /// \brief Returns true if the edge (i, j) is a reverse edge.
            bool isReverseEdge(unsigned i, unsigned j); 

            /// \brief Returns the unsigned id of the vertex \p s;
            unsigned getUId(std::string s);
            /// \brief Returns the std::string id of the vertex whose uid is \p i;
            std::string getSId(unsigned i);
    
            /// \brief Builds the reverse graph. It contains all the reverse
            /// edges that are not already in.
            void buildReverseGraph(); 

            /// \brief Encapsulates the creation of a new Graph.
            static std::unique_ptr<Graph> Create(unsigned n);

            /// \brief Parses the file \p filename into a Graph representation.
            static std::unique_ptr<Graph> Read(std::string filepath);

            /// \brief Parses the string \p graphStr into a Graph representation.
            static std::unique_ptr<Graph> ReadString(std::string graphStr);
    };
}

#endif
