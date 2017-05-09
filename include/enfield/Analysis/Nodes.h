#ifndef __EFD_NODES_H__
#define __EFD_NODES_H__

#include "enfield/Support/WrapperVal.h"

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
                K_LIST,
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

            /// \brief Returns a std::string representation of the operation.
            virtual std::string getOperation() const;
            /// \brief Returns the kind of this node.
            virtual Kind getKind() const = 0;
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
            /// \brief Creates a new instance of this node.
            static NodeRef create(Type t, NodeRef idNode, NodeRef sizeNode);
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
            /// \brief Creates a new instance of this node.
            static NodeRef create(NodeRef idNode, NodeRef aNode, NodeRef qaNode, NodeRef gopNode);
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
            /// \brief Creates a new instance of this node.
            static NodeRef create(NodeRef idNode, NodeRef aNode, NodeRef qaNode);
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
            /// \brief Creates a new instance of this node.
            static NodeRef create(NodeRef qNode, NodeRef cNode);
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
            /// \brief Creates a new instance of this node.
            static NodeRef create(NodeRef qaNode);
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
            /// \brief Creates a new instance of this node.
            static NodeRef create(NodeRef qaNode);
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
            /// \brief Creates a new instance of this node.
            static NodeRef create(NodeRef idNode, NodeRef aNode, NodeRef qaNode);
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
            /// \brief Creates a new instance of this node.
            static NodeRef create(OpType t, NodeRef lhsNode, NodeRef rhsNode);
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
            /// \brief Creates a new instance of this node.
            static NodeRef create(UOpType t, NodeRef oNode);
    };

    /// \brief Node for id references (register specific positions).
    class NDIdRef : public Node {
        private:
            enum ChildType {
                I_ID = 0,
                I_N
            };

            NDIdRef(NodeRef idNode, NodeRef sizeNode);

        public:
            Kind getKind() const override;
            std::string toString(bool pretty = false) const override;

            /// \brief Returns the type of this class.
            static Kind GetKind();
            /// \brief Creates a new instance of this node.
            static NodeRef create(NodeRef idNode, NodeRef sizeNode);
    };

    /// \brief Base class for list of nodes.
    class NDList : public Node {
        private:
            NDList();

        protected:
            NDList(Kind k);

        public:
            Kind getKind() const override;
            std::string toString(bool pretty = false) const override;

            /// \brief Appends a child to the end of the list.
            void addChild(NodeRef child);

            /// \brief Returns the type of this class.
            static Kind GetKind();
            /// \brief Creates a new instance of this node.
            static NodeRef create();
    };

    /// \brief Node for list of qubit operation sequences.
    class NDGOpList : public NDList {
        private:
            NDGOpList();

        public:
            Kind getKind() const override;
            std::string toString(bool pretty = false) const override;

            /// \brief Returns the type of this class.
            static Kind GetKind();
            /// \brief Creates a new instance of this node.
            static NodeRef create();
    };

    /// \brief Node for literal types.
    template <typename T>
        class NDValue : public Node {
            private:
                T mVal;

                NDValue(T val);

            public:
                typedef std::unique_ptr< NDValue<T> > NDRef;

                /// \brief Returns a copy to the setted value.
                T getVal() const;
                Kind getKind() const override;
                std::string getOperation() const override;
                std::string toString(bool pretty = false) const override;

                /// \brief Returns the type of this class.
                static Kind GetKind();
                /// \brief Creates a new instance of this node.
                static NodeRef create(T val);
        };

    template class NDValue<IntVal>;
    template <> NDValue<IntVal>::NDValue(IntVal val);
    template <> Node::Kind NDValue<IntVal>::GetKind();
    template <> Node::Kind NDValue<IntVal>::getKind() const;

    template class NDValue<RealVal>;
    template <> NDValue<RealVal>::NDValue(RealVal val);
    template <> Node::Kind NDValue<RealVal>::GetKind();
    template <> Node::Kind NDValue<RealVal>::getKind() const;

    template class NDValue<std::string>;
    template <> NDValue<std::string>::NDValue(std::string val);
    template <> Node::Kind NDValue<std::string>::GetKind();
    template <> Node::Kind NDValue<std::string>::getKind() const;
    template <> std::string NDValue<std::string>::getOperation() const;
    template <> std::string NDValue<std::string>::toString(bool pretty) const;

    typedef NDValue<IntVal> NDInt;
    typedef NDValue<RealVal> NDReal;
    typedef NDValue<std::string> NDId;
};

// -------------- Literal -----------------
template <typename T>
T efd::NDValue<T>::getVal() const {
    return mVal;
}

template <typename T>
std::string efd::NDValue<T>::getOperation() const {
    return std::to_string(mVal);
}

template <typename T>
std::string efd::NDValue<T>::toString(bool pretty) const {
    return std::to_string(mVal);
}

template <typename T>
efd::Node::NodeRef efd::NDValue<T>::create(T val) {
    return NodeRef(new NDValue<T>(val));
}

#endif
