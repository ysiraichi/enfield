#ifndef __EFD_ARCH_GRAPH_H__
#define __EFD_ARCH_GRAPH_H__

#include "enfield/Support/Graph.h"
#include "enfield/Analysis/Nodes.h"

namespace efd {
    /// \brief This is the base class for the architectures that this project will
    /// be supporting.
    class ArchGraph : public Graph {
        public:
            typedef ArchGraph* Ref;
            typedef std::unique_ptr<ArchGraph> uRef;
            typedef std::shared_ptr<ArchGraph> sRef;

            typedef std::unordered_map<std::string, unsigned> RegsVector;
            typedef RegsVector::iterator RegsIterator;

        protected:
            RegsVector mRegs;
            std::vector<Node::uRef> mNodes;

            std::vector<std::string> mId; 
            std::unordered_map<std::string, unsigned> mStrToId;

            bool mGeneric;
            unsigned mVID;

            ArchGraph(unsigned n, bool isGeneric = true);

        public:
            /// \brief Gets the node corresponding to the uid.
            Node::Ref getNode(unsigned i) const;

            /// \brief Creates a string vertex and puts it in the vector.
            unsigned putVertex(std::string s);
            /// \brief Creates a node vertex and puts it in the vector.
            unsigned putVertex(Node::uRef node);

            /// \brief Register the register.
            void putReg(std::string id, std::string size);

            /// \brief Returns the unsigned id of the vertex \p s;
            unsigned getUId(std::string s);
            /// \brief Returns the std::string id of the vertex whose uid is \p i;
            std::string getSId(unsigned i);

            /// \brief Returns true if the edge (i, j) is a reverse edge.
            /// i.e.: if (i, j) is not in the graph, but (j, i) is.
            bool isReverseEdge(unsigned i, unsigned j); 
            /// \brief Returns true if this is a generic architechture graph,
            /// i.e.: it was not created by any of the architechtures compiled within
            /// the program.
            bool isGeneric();

            /// \brief The begin iterator for the \p mRegs.
            RegsIterator reg_begin();
            /// \brief The end iterator for the \p mRegs.
            RegsIterator reg_end();

            /// \brief Encapsulates the creation of a new ArchGraph.
            static uRef Create(unsigned n);

            /// \brief Parses the file \p filename into a ArchGraph representation.
            static uRef Read(std::string filepath);

            /// \brief Parses the string \p graphStr into a ArchGraph representation.
            static uRef ReadString(std::string graphStr);
    };
}

#endif
