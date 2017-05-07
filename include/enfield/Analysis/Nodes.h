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

        protected:
            bool mIsEmpty;
            /// \brief The childrem nodes.
            std::vector<NodeRef> mChild;

            /// \brief Constructs the node, initially empty (with no information).
            Node(bool empty);

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

            std::string getOperation() const override;
            std::string toString(bool pretty = false) const override;
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
            std::string getOperation() const override;
            std::string toString(bool pretty = false) const override;
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
            std::string getOperation() const override;
            std::string toString(bool pretty = false) const override;
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
            NDQOp(QOpType type);

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
            std::string getOperation() const override;
            std::string toString(bool pretty = false) const override;
    };

    /// \brief NDQOp specialized for reset operation.
    class NDQOpReset : public NDQOp {
        private:
            enum ChildType {
                I_ONLY = 0
            };

        public:
            NDQOpReset(NodeRef qaNode);
            std::string getOperation() const override;
            std::string toString(bool pretty = false) const override;
    };

    /// \brief NDQOp specialized for barrier operation.
    class NDQOpBarrier : public NDQOp {
        private:
            enum ChildType {
                I_ONLY = 0
            };

        public:
            NDQOpBarrier(NodeRef qaNode);
            std::string getOperation() const override;
            std::string toString(bool pretty = false) const override;
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

            std::string getOperation() const override;
            std::string toString(bool pretty = false) const override;
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

            std::string getOperation() const override;
            std::string toString(bool pretty = false) const override;
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

            std::string getOperation() const override;
            std::string toString(bool pretty = false) const override;
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
            std::string toString(bool pretty = false) const override;
    };

    /// \brief Base class for list of nodes.
    class NDList : public Node {
        public:
            NDList();
            /// \brief Appends a child to the end of the list.
            void addChild(NodeRef child);
    };

    /// \brief Node for arg lists.
    class NDArgList : public NDList {
        public:
            std::string toString(bool pretty = false) const override;
    };

    /// \brief Node for list of qubit operation sequences.
    class NDGOpList : public NDList {
        public:
            std::string toString(bool pretty = false) const override;
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
            std::string getOperation() const override;
    };

    template class NDLiteral<int>;
    template class NDLiteral<DoubleVal>;
    template class NDLiteral<std::string>;

    typedef NDLiteral<int> NDInt;
    typedef NDLiteral<DoubleVal> NDReal;
    typedef NDLiteral<std::string> NDId;

};

// -------------- Literal -----------------
template <typename T>
efd::NDLiteral<T>::NDLiteral(T val) : Node(false), mVal(val) {
}

template <typename T>
T efd::NDLiteral<T>::getVal() const {
    return mVal;
}

template <typename T>
std::string efd::NDLiteral<T>::getOperation() const {
    return std::to_string(mVal);
}

#endif
