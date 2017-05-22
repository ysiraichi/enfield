#ifndef __EFD_GRAPH_H__
#define __EFD_GRAPH_H__

#include <vector>
#include <set>

namespace efd {
    /// \brief Graph representation.
    class Graph {
        private:
            unsigned mN;
            std::vector<std::set<unsigned>> mSuccessors;
            std::vector<std::set<unsigned>> mPredecessors;
            std::set<std::pair<unsigned, unsigned>> mReverseEdges;
    
        public:
            Graph(unsigned n);
    
            /// \brief
            unsigned inDegree(unsigned i) const;
            unsigned outDegree(unsigned i) const; 
            unsigned size() const;
    
            std::set<unsigned>& succ(unsigned i);
            std::set<unsigned>& pred(unsigned i); 
    
            bool hasEdge(unsigned i, unsigned j); 
    
            void putEdge(unsigned i, unsigned j);
    
            bool isReverseEdge(unsigned i, unsigned j); 
    
            void buildReverseGraph(); 
    
            void print(); 
    };
}

#endif
