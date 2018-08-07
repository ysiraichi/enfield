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
            Xbit(uint32_t realId, uint32_t qubits, uint32_t cbits);

            bool isQuantum();
            bool isClassic();

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
        public:
            /// \brief Representation of a quantum operation.
            struct CircuitNode {
                public:
                    typedef CircuitNode* Ref;
                    typedef std::unique_ptr<CircuitNode> uRef;
                    typedef std::shared_ptr<CircuitNode> sRef;

                private:
                    enum class Type { INPUT, OUTPUT, GATE };

                    Type mType;
                    Node::Ref mNode;
                    std::unordered_map
                        <uint32_t, std::pair<CircuitNode::sRef, CircuitNode::sRef>>
                        mStepMap;

                    CircuitNode(Type type);

                public:
                    /// \brief Returns the \em Node::Ref associated with this circuit node.
                    Node::Ref node();

                    /// \brief Returns the number of \p Xbit's in this node.
                    uint32_t numberOfXbits();

                    /// \brief True if this node has reached the end (output node).
                    bool isOutputNode();
                    /// \brief True if this node is in the beginning (input node).
                    bool isInputNode();
                    /// \brief True if this node is in the middle (gate node).
                    bool isGateNode();

                    /// \brief Returns the \p Xbits in this node.
                    std::vector<Xbit> getXbits(uint32_t qubits, uint32_t cbits);
                    /// \brief Returns the \p Xbit ids in this node.
                    std::vector<uint32_t> getXbitsId();

                    friend class CircuitGraph;
            };

        private:
            bool mInit;
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
                    bool next(uint32_t id);
                    /// \brief Retreats the bit \p xbit.
                    bool back(Xbit xbit);
                    bool back(uint32_t id);
                    /// \brief Returns the \p Node::Ref for the bit \p xbit.
                    Node::Ref get(Xbit xbit);
                    Node::Ref get(uint32_t id);

                    CircuitNode::sRef operator[](Xbit xbit);
                    CircuitNode::sRef operator[](uint32_t id);

                    friend class CircuitGraph;
            };

            CircuitGraph();
            CircuitGraph(uint32_t qubits, uint32_t cbits);

            /// \brief Initializes the CircuitGraph.
            void init(uint32_t qubits, uint32_t cbits);
            /// \brief Checks if the CircuitGraph is initialized. Exits with error if not.
            void checkInitialized();

            /// \brief Returns the number of qubits.
            uint32_t getQSize() const;
            /// \brief Returns the number of cbits.
            uint32_t getCSize() const;
            /// \brief Returns the number of bits.
            uint32_t size() const;

            /// \brief Appends a node to the bits \p xbits.
            void append(std::vector<Xbit> xbits, Node::Ref node);

            /// \brief Builds an iterator instance for this \p CircuitGraph.
            Iterator build_iterator();
    };
}

#endif
