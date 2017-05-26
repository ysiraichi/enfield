#ifndef __EFD_ARCH_GRAPH_H__
#define __EFD_ARCH_GRAPH_H__

#include "enfield/Support/Graph.h"
#include "enfield/Analysis/Nodes.h"

namespace efd {
    /// \brief This is the base class for the architectures that this project will
    /// be supporting.
    class ArchGraph : public Graph {
        public:
            typedef std::unordered_map<std::string, unsigned> RegsVector;
            typedef RegsVector::iterator RegsIterator;

        protected:
            std::vector<NodeRef> mNodes;
            RegsVector mRegs;

            ArchGraph(unsigned n);

            void preprocessVertexString(unsigned i, std::string s);

        public:
            /// \brief Gets the node corresponding to the uid.
            NodeRef getNode(unsigned i) const;

            /// \brief Creates a node and puts it in the vector.
            unsigned putVertex(std::string s);

            /// \brief The begin iterator for the \p mRegs.
            RegsIterator reg_begin();
            /// \brief The end iterator for the \p mRegs.
            RegsIterator reg_end();

            /// \brief Encapsulates the creation of a new ArchGraph.
            static std::unique_ptr<ArchGraph> Create(unsigned n);

            /// \brief Parses the file \p filename into a ArchGraph representation.
            static std::unique_ptr<ArchGraph> Read(std::string filepath);

            /// \brief Parses the string \p graphStr into a ArchGraph representation.
            static std::unique_ptr<ArchGraph> ReadString(std::string graphStr);

            static bool ClassOf(const Graph* graph);
    };
}

#endif
