#ifndef __EFD_NODES_H__
#define __EFD_NODES_H__

#include "enfield/Support/DoubleVal.h"

#include <iostream>
#include <vector>
#include <memory>

namespace efd {

    /// \brief Base class for AST nodes.
    class Node {
        public:
            typedef std::unique_ptr<Node> NodeRef;
            typedef std::vector<NodeRef>::iterator Iterator;
            typedef std::vector<NodeRef>::const_iterator ConstIterator;

            enum Kind {
                K_DECL,
                K_GATE_DECL,
                K_GATE_OPAQUE,
                K_QOP_MEASURE,
                K_QOP_RESET,
                K_QOP_BARRIER,
                K_QOP_GENERIC,
                K_BINOP,
                K_UNARYOP,
                K_ID_REF,
                K_ARG_LIST,
                K_GOP_LIST,
                K_LIT_INT,
                K_LIT_REAL,
                K_LIT_STRING
            };

        protected:
            /// \brief The kind of the node.
            Kind mK;
            /// \brief Holds whether this node has no information.
            bool mIsEmpty;
            /// \brief The childrem nodes.
            std::vector<NodeRef> mChild;

            /// \brief Constructs the node, initially empty (with no information).
            Node(Kind k, bool empty = false);

        public:
            /// \brief Returns a iterator to the beginning of the vector.
            Iterator begin();
            /// \brief Returns a iterator to the end of the vector.
            Iterator end();

            ConstIterator begin() const;
            ConstIterator end() const;

            /// \brief Prints from this node, recursively to \p O.
            void print(std::ostream& O = std::cout, bool pretty = false);
            /// \brief Prints to the standard output.
            void print(bool pretty = false);

            /// \brief Returns whether this node has any information.
            bool isEmpty() const;

            /// \brief Returns the kind of this node.
            virtual Kind getKind() const = 0;
            /// \brief Returns a std::string representation of the operation.
            virtual std::string getOperation() const = 0;
            /// \brief Returns a std::string representation of this Node and its childrem.
            virtual std::string toString(bool pretty = false) const = 0;
    };

    /// \brief Node for declaration of registers (concrete and quantum).
    class NDDecl : public Node {
        public:
            /// \brief The possible types of declaration.
            enum Type {
                CONCRETE,
                QUANTUM
            };

        private:
            enum ChildType {
                I_ID = 0,
                I_SIZE
            };

            Type mT;

        public:
            NDDecl(Type t, NodeRef idNode, NodeRef sizeNode);

            /// \brief Returns true if it is a concrete register declaration.
            bool isCReg() const;
            /// \brief Returns true if it is a quantum register declaration.
            bool isQReg() const;

            Kind getKind() const override;
            std::string getOperation() const override;
            std::string toString(bool pretty = false) const override;

            /// \brief Returns the type of this class.
            static Kind GetKind();
    };

    /// \brief Node for declaration of quantum gates.
    class NDGateDecl : public Node {
        private:
            enum ChildType {
                I_ID = 0,
                I_ARGS,
                I_QARGS,
                I_GOPLIST
            };

        public:
            NDGateDecl(NodeRef idNode, NodeRef aNode, NodeRef qaNode, NodeRef gopNode);
            Kind getKind() const override;
            std::string getOperation() const override;
            std::string toString(bool pretty = false) const override;

            /// \brief Returns the type of this class.
            static Kind GetKind();
    };

    /// \brief Node for declaration of opaque quantum gates.
    class NDOpaque : public Node {
        private:
            enum ChildType {
                I_ID = 0,
                I_ARGS,
                I_QARGS
            };

        public:
            NDOpaque(NodeRef idNode, NodeRef aNode, NodeRef qaNode);
            Kind getKind() const override;
            std::string getOperation() const override;
            std::string toString(bool pretty = false) const override;

            /// \brief Returns the type of this class.
            static Kind GetKind();
    };

    /// \brief Base node for quantum operations.
    class NDQOp : public Node {
        public:
            /// \brief The types of quantum operations.
            enum QOpType {
                QOP_RESET,
                QOP_BARRIER,
                QOP_MEASURE,
                QOP_U,
                QOP_CX,
                QOP_GENERIC
            };

        private:

            QOpType mT;

        public:
            NDQOp(Kind k, QOpType type);

            /// \brief Returns the type o the operation of this node.
            QOpType getQOpType() const;

            /// \brief Returns true if this is a reset node.
            virtual bool isReset() const;
            /// \brief Returns true if this is a barrier node.
            virtual bool isBarrier() const;
            /// \brief Returns true if this is a measure node.
            virtual bool isMeasure() const;
            /// \brief Returns true if this is a u node.
            virtual bool isU() const;
            /// \brief Returns true if this is a generic node.
            virtual bool isGeneric() const;
    };

    /// \brief NDQOp specialized for measure operation.
    class NDQOpMeasure : public NDQOp {
        private:
            enum ChildType {
                I_QBIT = 0,
                I_CBIT
            };

        public:
            NDQOpMeasure(NodeRef qNode, NodeRef cNode);
            Kind getKind() const override;
            std::string getOperation() const override;
            std::string toString(bool pretty = false) const override;

            /// \brief Returns the type of this class.
            static Kind GetKind();
    };

    /// \brief NDQOp specialized for reset operation.
    class NDQOpReset : public NDQOp {
        private:
            enum ChildType {
                I_ONLY = 0
            };

        public:
            NDQOpReset(NodeRef qaNode);
            Kind getKind() const override;
            std::string getOperation() const override;
            std::string toString(bool pretty = false) const override;

            /// \brief Returns the type of this class.
            static Kind GetKind();
    };

    /// \brief NDQOp specialized for barrier operation.
    class NDQOpBarrier : public NDQOp {
        private:
            enum ChildType {
                I_ONLY = 0
            };

        public:
            NDQOpBarrier(NodeRef qaNode);
            Kind getKind() const override;
            std::string getOperation() const override;
            std::string toString(bool pretty = false) const override;

            /// \brief Returns the type of this class.
            static Kind GetKind();
    };

    /// \brief NDQOp specialized for generic operation.
    class NDQOpGeneric : public NDQOp {
        private:
            enum ChildType {
                I_ID = 0,
                I_ARGS,
                I_QARGS
            };

        public:
            NDQOpGeneric(NodeRef idNode, NodeRef aNode, NodeRef qaNode);

            Kind getKind() const override;
            std::string getOperation() const override;
            std::string toString(bool pretty = false) const override;

            /// \brief Returns the type of this class.
            static Kind GetKind();
    };

    /// \brief Binary operation node.
    class NDBinOp : public Node {
        private:
            enum ChildType {
                I_LHS = 0,
                I_RHS
            };

        public:
            /// \brief The possible binary operations.
            enum OpType {
                OP_ADD = 0,
                OP_SUB,
                OP_MUL,
                OP_DIV,
                OP_POW
            };

        private:
            OpType mT;

        public:
            NDBinOp(OpType t, NodeRef lhsNode, NodeRef rhsNode);

            /// \brief Returns the type of the binary operation of this node.
            OpType getOpType() const;

            /// \brief Returns whether this is an add operation.
            bool isAdd() const;
            /// \brief Returns whether this is an sub operation.
            bool isSub() const;
            /// \brief Returns whether this is an mul operation.
            bool isMul() const;
            /// \brief Returns whether this is an div operation.
            bool isDiv() const;
            /// \brief Returns whether this is an pow operation.
            bool isPow() const;

            Kind getKind() const override;
            std::string getOperation() const override;
            std::string toString(bool pretty = false) const override;

            /// \brief Returns the type of this class.
            static Kind GetKind();
    };

    /// \brief Unary operation node.
    class NDUnaryOp : public Node {
        private:
            enum ChildType {
                I_ONLY = 0
            };

        public:
            /// \brief Unary operations available.
            enum UOpType {
                UOP_SIN = 0,
                UOP_COS,
                UOP_TAN,
                UOP_EXP,
                UOP_LN,
                UOP_SQRT,
                UOP_NEG
            };

        private:
            UOpType mT;

        public:
            NDUnaryOp(UOpType t, NodeRef oNode);

            /// \brief Returns the unary operation type.
            UOpType getUOpType() const;

            /// \brief Returns whether this is an neg operation.
            bool isNeg() const;
            /// \brief Returns whether this is an sin operation.
            bool isSin() const;
            /// \brief Returns whether this is an cos operation.
            bool isCos() const;
            /// \brief Returns whether this is an tan operation.
            bool isTan() const;
            /// \brief Returns whether this is an exp operation.
            bool isExp() const;
            /// \brief Returns whether this is an ln operation.
            bool isLn() const;
            /// \brief Returns whether this is an sqrt operation.
            bool isSqrt() const;

            Kind getKind() const override;
            std::string getOperation() const override;
            std::string toString(bool pretty = false) const override;

            /// \brief Returns the type of this class.
            static Kind GetKind();
    };

    /// \brief Node for id references (register specific positions).
    class NDIdRef : public Node {
        private:
            enum ChildType {
                I_ID = 0,
                I_N
            };

        public:
            NDIdRef(NodeRef idNode, NodeRef sizeNode);

            Kind getKind() const override;
            std::string toString(bool pretty = false) const override;

            /// \brief Returns the type of this class.
            static Kind GetKind();
    };

    /// \brief Base class for list of nodes.
    class NDList : public Node {
        public:
            NDList(Kind k);
            /// \brief Appends a child to the end of the list.
            void addChild(NodeRef child);
    };

    /// \brief Node for arg lists.
    class NDArgList : public NDList {
        public:
            NDArgList();

            Kind getKind() const override;
            std::string toString(bool pretty = false) const override;

            /// \brief Returns the type of this class.
            static Kind GetKind();
    };

    /// \brief Node for list of qubit operation sequences.
    class NDGOpList : public NDList {
        public:
            NDGOpList();

            Kind getKind() const override;
            std::string toString(bool pretty = false) const override;

            /// \brief Returns the type of this class.
            static Kind GetKind();
    };

    /// \brief Node for literal types.
    template <typename T>
        class NDLiteral : public Node {
            private:
                T mVal;

            public:
                NDLiteral(T val);

                /// \brief Returns a copy to the setted value.
                T getVal() const;
                Kind getKind() const override;
                std::string getOperation() const override;

                /// \brief Returns the type of this class.
                static Kind GetKind();
        };

    template class NDLiteral<int>;
    template <> NDLiteral<int>::NDLiteral(int val);
    template <> Node::Kind NDLiteral<int>::GetKind();
    template <> Node::Kind NDLiteral<int>::getKind() const;

    template class NDLiteral<DoubleVal>;
    template <> NDLiteral<DoubleVal>::NDLiteral(DoubleVal val);
    template <> Node::Kind NDLiteral<DoubleVal>::GetKind();
    template <> Node::Kind NDLiteral<DoubleVal>::getKind() const;

    template class NDLiteral<std::string>;
    template <> NDLiteral<std::string>::NDLiteral(std::string val);
    template <> Node::Kind NDLiteral<std::string>::GetKind();
    template <> Node::Kind NDLiteral<std::string>::getKind() const;

    typedef NDLiteral<int> NDInt;
    typedef NDLiteral<DoubleVal> NDReal;
    typedef NDLiteral<std::string> NDId;

    /// \brief Wrapper function that creates an instance of NodeRef.
    template <typename T, typename... Tn>
        Node::NodeRef CreateNode(Tn... args);
};

// -------------- Literal -----------------
template <> efd::NDLiteral<int>::NDLiteral(int val) : Node(K_LIT_INT), mVal(val) {}
template <> efd::Node::Kind efd::NDLiteral<int>::GetKind() { return K_LIT_INT; }
template <> efd::Node::Kind efd::NDLiteral<int>::getKind() const { return K_LIT_INT; }

template <> efd::NDLiteral<efd::DoubleVal>::NDLiteral(efd::DoubleVal val) : Node(K_LIT_REAL), mVal(val) {}
template <> efd::Node::Kind efd::NDLiteral<efd::DoubleVal>::GetKind() { return K_LIT_REAL; }
template <> efd::Node::Kind efd::NDLiteral<efd::DoubleVal>::getKind() const { return K_LIT_REAL; }

template <> efd::NDLiteral<std::string>::NDLiteral(std::string val) : Node(K_LIT_STRING), mVal(val) {}
template <> efd::Node::Kind efd::NDLiteral<std::string>::GetKind() { return K_LIT_STRING; }
template <> efd::Node::Kind efd::NDLiteral<std::string>::getKind() const { return K_LIT_STRING; }

template <typename T>
T efd::NDLiteral<T>::getVal() const {
    return mVal;
}

template <typename T>
std::string efd::NDLiteral<T>::getOperation() const {
    return std::to_string(mVal);
}

// -------------- Create -----------------
template <typename T, typename... Tn>
efd::Node::NodeRef efd::CreateNode(Tn... args) {
    Node::NodeRef ref(new T(args...));
    return ref;
}


#endif
