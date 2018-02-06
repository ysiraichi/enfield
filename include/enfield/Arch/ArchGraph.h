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

            typedef std::unordered_map<std::string, uint32_t> RegsVector;
            typedef RegsVector::iterator RegsIterator;

        protected:
            RegsVector mRegs;
            std::vector<Node::uRef> mNodes;

            std::vector<std::string> mId; 
            std::unordered_map<std::string, uint32_t> mStrToId;

            bool mGeneric;
            uint32_t mVID;

            ArchGraph(uint32_t n, bool isGeneric = true);
            std::string vertexToString(uint32_t i) const override;

        public:
            /// \brief Gets the node corresponding to the uid.
            Node::Ref getNode(uint32_t i) const;

            /// \brief Creates a string vertex and puts it in the vector.
            uint32_t putVertex(std::string s);
            /// \brief Creates a node vertex and puts it in the vector.
            uint32_t putVertex(Node::uRef node);

            /// \brief Register the register.
            void putReg(std::string id, std::string size);

            /// \brief Returns the uint32_t id of the vertex \p s.
            uint32_t getUId(std::string s);
            /// \brief Returns true if this architecture has a vertex whose string
            /// representation is \p s.
            bool hasSId(std::string s) const;
            /// \brief Returns the std::string id of the vertex whose uid is \p i.
            std::string getSId(uint32_t i);

            /// \brief Returns true if the edge (i, j) is a reverse edge.
            /// i.e.: if (i, j) is not in the graph, but (j, i) is.
            bool isReverseEdge(uint32_t i, uint32_t j); 
            /// \brief Returns true if this is a generic architechture graph,
            /// i.e.: it was not created by any of the architechtures compiled within
            /// the program.
            bool isGeneric();

            /// \brief The begin iterator for the \p mRegs.
            RegsIterator reg_begin();
            /// \brief The end iterator for the \p mRegs.
            RegsIterator reg_end();

            /// \brief Returns true if \p g is of this type.
            static bool ClassOf(const Graph* g);

            /// \brief Encapsulates the creation of a new ArchGraph.
            static uRef Create(uint32_t n);
            /// \brief Parses the file \p filename into a ArchGraph representation.
            static uRef Read(std::string filepath);
            /// \brief Parses the string \p graphStr into a ArchGraph representation.
            static uRef ReadString(std::string graphStr);
    };
}

#endif
