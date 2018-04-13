#ifndef __EFD_CIRCUIT_GRAPH_H__
#define __EFD_CIRCUIT_GRAPH_H__

#include "enfield/Analysis/Nodes.h"

#include <unordered_map>

namespace efd {
    /// \brief Represents the id of one Quantum or Classical bit.
    class Xbit {
        public:
            enum class Type { QUANTUM, CLASSIC };

            Type mType;
            uint32_t mId;

        private:
            Xbit(Type t, uint32_t id);

        public:
            /// \brief Gets the real id for this bit.
            ///
            /// If it is a classical bit, we sum \p qubits to it. 
            uint32_t getRealId(uint32_t qubits, uint32_t cbits);

            /// \brief Creates a quantum \p Xbit with \p id.
            static Xbit Q(uint32_t id);
            /// \brief Creates a classical \p Xbit with \p id.
            static Xbit C(uint32_t id);
    };

    /// \brief The Circuit representation of the \em QModule.
    class CircuitGraph {
        private:
            /// \brief Representation of a quantum operation.
            struct CircuitNode {
                enum class Type { INPUT, OUTPUT, GATE };

                typedef CircuitNode* Ref;
                typedef std::unique_ptr<CircuitNode> uRef;
                typedef std::shared_ptr<CircuitNode> sRef;

                Type mType;
                Node::Ref mNode;
                std::unordered_map
                    <uint32_t, std::pair<CircuitNode::sRef, CircuitNode::sRef>>
                    mStepMap;

                CircuitNode(Type type);
            };

            uint32_t mQubits;
            uint32_t mCbits;
            std::vector<CircuitNode::sRef> mGraphHead;
            std::vector<CircuitNode::sRef> mGraphTail;

        public:
            /// \brief Abstracts the iteration of the \em CircuitGraph.
            class Iterator {
                private:
                    uint32_t mQubits;
                    uint32_t mCbits;
                    std::vector<CircuitNode::sRef> mPtr;

                    Iterator(uint32_t qubits,
                             uint32_t cbits,
                             std::vector<CircuitNode::sRef> ptr);

                public:
                    Iterator();

                    /// \brief Advances the bit \p xbit.
                    bool next(Xbit xbit);
                    /// \brief Retreats the bit \p xbit.
                    bool back(Xbit xbit);
                    /// \brief Returns the \p Node::Ref for the bit \p xbit.
                    Node::Ref get(Xbit xbit);

                    /// \brief True if all bits have reached the end.
                    bool endOfCircuit();
                    /// \brief True if \p xbit has reached the end.
                    bool endOfCircuit(Xbit xbit);
                    /// \brief True if \p xbit is in the beginning.
                    bool beginningOfCircuit(Xbit xbit);

                    friend class CircuitGraph;
            };

            CircuitGraph(uint32_t qubits, uint32_t cbits);

            /// \brief Appends a node to the bits \p xbits.
            void append(std::vector<Xbit> xbits, Node::Ref node);

            /// \brief Builds an iterator instance for this \p CircuitGraph.
            Iterator build_iterator();
    };
}

#endif
