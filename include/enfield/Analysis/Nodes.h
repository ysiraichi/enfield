#ifndef __EFD_NODES_H__
#define __EFD_NODES_H__

#include "enfield/Support/WrapperVal.h"

#include <iostream>
#include <vector>
#include <memory>

namespace efd {
    class Node;
    class NodeVisitor;

    class NDQasmVersion;
    class NDInclude;
    class NDDecl;
    class NDGateDecl;
    class NDOpaque;
    class NDQOp;
    class NDQOpMeasure;
    class NDQOpReset;
    class NDQOpBarrier;
    class NDQOpCX;
    class NDQOpU;
    class NDQOpGeneric;
    class NDBinOp;
    class NDUnaryOp;
    class NDIdRef;
    class NDList;
    class NDStmtList;
    class NDGOpList;
    class NDIfStmt;

    typedef Node* NodeRef;

    /// \brief Base class for AST nodes.
    class Node {
        public:
            typedef std::vector<NodeRef>::iterator Iterator;
            typedef std::vector<NodeRef>::const_iterator ConstIterator;

            enum Kind {
                K_QASM_VERSION,
                K_DECL,
                K_GATE_DECL,
                K_GATE_OPAQUE,
                K_QOP_MEASURE,
                K_QOP_RESET,
                K_QOP_BARRIER,
                K_QOP_GENERIC,
                K_QOP_CX,
                K_QOP_U,
                K_BINOP,
                K_UNARYOP,
                K_ID_REF,
                K_LIST,
                K_STMT_LIST,
                K_GOP_LIST,
                K_IF_STMT,
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
            Node(Kind k, unsigned size = 0, bool empty = false);

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
            /// \brief Returns the number of childrem of this node. 
            virtual unsigned getChildNumber() const;
            /// \brief Returns the kind of this node.
            virtual Kind getKind() const = 0;
            /// \brief Returns a std::string representation of this Node and its childrem.
            virtual std::string toString(bool pretty = false) const = 0;
            /// \brief Used by visitor classes.
            virtual void apply(NodeVisitor* visitor) = 0;

    };

    /// \brief Node for literal types.
    template <typename T>
        class NDValue : public Node {
            private:
                T mVal;

                NDValue(T val);

            public:
                typedef std::shared_ptr< NDValue<T> > NDRef;

                /// \brief Returns a copy to the setted value.
                T getVal() const;

                Kind getKind() const override;

                std::string getOperation() const override;
                std::string toString(bool pretty = false) const override;

                unsigned getChildNumber() const override;

                void apply(NodeVisitor* visitor) override;

                /// \brief Returns whether the \p node is an instance of this class.
                static bool ClassOf(const NodeRef node);
                /// \brief Creates a new instance of this node.
                static NodeRef Create(T val);
        };

    template class NDValue<IntVal>;
    template <> NDValue<IntVal>::NDValue(IntVal val);
    template <> bool NDValue<IntVal>::ClassOf(const NodeRef node);
    template <> Node::Kind NDValue<IntVal>::getKind() const;
    template <> void NDValue<IntVal>::apply(NodeVisitor* visitor);

    template class NDValue<RealVal>;
    template <> NDValue<RealVal>::NDValue(RealVal val);
    template <> bool NDValue<RealVal>::ClassOf(const NodeRef node);
    template <> Node::Kind NDValue<RealVal>::getKind() const;
    template <> void NDValue<RealVal>::apply(NodeVisitor* visitor);

    template class NDValue<std::string>;
    template <> NDValue<std::string>::NDValue(std::string val);
    template <> bool NDValue<std::string>::ClassOf(const NodeRef node);
    template <> Node::Kind NDValue<std::string>::getKind() const;
    template <> std::string NDValue<std::string>::getOperation() const;
    template <> std::string NDValue<std::string>::toString(bool pretty) const;
    template <> void NDValue<std::string>::apply(NodeVisitor* visitor);

    typedef NDValue<IntVal> NDInt;
    typedef NDValue<RealVal> NDReal;
    typedef NDValue<std::string> NDId;
    typedef NDValue<std::string> NDString;

    /// \brief Node that holds the current Qasm version and the
    /// rest of the program.
    class NDQasmVersion : public Node {
        private:
            enum ChildType {
                I_VERSION = 0,
                I_STMTS
            };

            NDQasmVersion(NodeRef vNode, NodeRef stmtsNode);

        public:
            /// \brief Gets the node that holds the version.
            NDReal* getVersion() const;
            /// \brief Gets the node that holds the statements.
            NDStmtList* getStatements() const;

            Kind getKind() const override;

            std::string getOperation() const override;
            std::string toString(bool pretty = false) const override;

            unsigned getChildNumber() const override;

            void apply(NodeVisitor* visitor) override;

            /// \brief Returns whether the \p node is an instance of this class.
            static bool ClassOf(const NodeRef node);
            /// \brief Creates a new instance of this node.
            static NodeRef Create(NodeRef vNode, NodeRef stmtsNode);
    };

    /// \brief Node used to parse another file.
    /// 
    /// The AST from this other file is a child of this node.
    class NDInclude : public Node {
        private:
            enum ChildType {
                I_FILE = 0,
                I_STMTS
            };

            NDInclude(NodeRef fNode, NodeRef stmtsNode);

        public:
            /// \brief Gets the node that holds the filename.
            NDString* getFilename() const;
            /// \brief Gets the node that holds the statements.
            NDStmtList* getStatements() const;

            Kind getKind() const override;

            std::string getOperation() const override;
            std::string toString(bool pretty = false) const override;

            unsigned getChildNumber() const override;

            void apply(NodeVisitor* visitor) override;

            /// \brief Returns whether the \p node is an instance of this class.
            static bool ClassOf(const NodeRef node);
            /// \brief Creates a new instance of this node.
            static NodeRef Create(NodeRef fNode, NodeRef stmtsNode);
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

            NDDecl(Type t, NodeRef idNode, NodeRef sizeNode);

        public:
            /// \brief Gets the id node.
            NDId* getId() const;
            /// \brief Gets the size node.
            NDInt* getSize() const;

            /// \brief Returns true if it is a concrete register declaration.
            bool isCReg() const;
            /// \brief Returns true if it is a quantum register declaration.
            bool isQReg() const;

            Kind getKind() const override;

            std::string getOperation() const override;
            std::string toString(bool pretty = false) const override;

            unsigned getChildNumber() const override;

            void apply(NodeVisitor* visitor) override;

            /// \brief Returns whether the \p node is an instance of this class.
            static bool ClassOf(const NodeRef node);
            /// \brief Creates a new instance of this node.
            static NodeRef Create(Type t, NodeRef idNode, NodeRef sizeNode);
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

            NDGateDecl(NodeRef idNode, NodeRef aNode, NodeRef qaNode, NodeRef gopNode);

        public:
            /// \brief Gets the id node.
            NDId* getId() const;
            /// \brief Gets the args node.
            NDList* getArgs() const;
            /// \brief Gets the qargs node.
            NDList* getQArgs() const;
            /// \brief Gets the goplist node.
            NDGOpList* getGOpList() const;

            Kind getKind() const override;

            std::string getOperation() const override;
            std::string toString(bool pretty = false) const override;

            unsigned getChildNumber() const override;

            void apply(NodeVisitor* visitor) override;

            /// \brief Returns whether the \p node is an instance of this class.
            static bool ClassOf(const NodeRef node);
            /// \brief Creates a new instance of this node.
            static NodeRef Create(NodeRef idNode, NodeRef aNode, NodeRef qaNode, NodeRef gopNode);
    };

    /// \brief Node for declaration of opaque quantum gates.
    class NDOpaque : public Node {
        private:
            enum ChildType {
                I_ID = 0,
                I_ARGS,
                I_QARGS
            };

            NDOpaque(NodeRef idNode, NodeRef aNode, NodeRef qaNode);

        public:
            /// \brief Gets the id node.
            NDId* getId() const;
            /// \brief Gets the args node.
            NDList* getArgs() const;
            /// \brief Gets the qargs node.
            NDList* getQArgs() const;

            Kind getKind() const override;

            std::string getOperation() const override;
            std::string toString(bool pretty = false) const override;

            unsigned getChildNumber() const override;

            void apply(NodeVisitor* visitor) override;

            /// \brief Returns whether the \p node is an instance of this class.
            static bool ClassOf(const NodeRef node);
            /// \brief Creates a new instance of this node.
            static NodeRef Create(NodeRef idNode, NodeRef aNode, NodeRef qaNode);
    };

    /// \brief Base node for quantum operations.
    class NDQOp : public Node {
        protected:
            NDQOp(Kind k, unsigned size = 0);

        public:
            /// \brief Returns true if this is a reset node.
            virtual bool isReset() const;
            /// \brief Returns true if this is a barrier node.
            virtual bool isBarrier() const;
            /// \brief Returns true if this is a measure node.
            virtual bool isMeasure() const;
            /// \brief Returns true if this is a u node.
            virtual bool isU() const;
            /// \brief Returns true if this is a cx node.
            virtual bool isCX() const;
            /// \brief Returns true if this is a generic node.
            virtual bool isGeneric() const;

            /// \brief Returns whether the \p node is an instance of this class.
            static bool ClassOf(const NodeRef node);
    };

    /// \brief NDQOp specialized for measure operation.
    class NDQOpMeasure : public NDQOp {
        private:
            enum ChildType {
                I_QBIT = 0,
                I_CBIT
            };

            NDQOpMeasure(NodeRef qNode, NodeRef cNode);

        public:
            /// \brief Gets the qbit node.
            NodeRef getQBit() const;
            /// \brief Gets the cbit node.
            NodeRef getCBit() const;

            Kind getKind() const override;

            std::string getOperation() const override;
            std::string toString(bool pretty = false) const override;

            unsigned getChildNumber() const override;

            void apply(NodeVisitor* visitor) override;

            /// \brief Returns whether the \p node is an instance of this class.
            static bool ClassOf(const NodeRef node);
            /// \brief Creates a new instance of this node.
            static NodeRef Create(NodeRef qNode, NodeRef cNode);
    };

    /// \brief NDQOp specialized for reset operation.
    class NDQOpReset : public NDQOp {
        private:
            enum ChildType {
                I_ONLY = 0
            };

            NDQOpReset(NodeRef qaNode);

        public:
            /// \brief Gets the quantum argument.
            NodeRef getQArg() const;

            Kind getKind() const override;

            std::string getOperation() const override;
            std::string toString(bool pretty = false) const override;

            unsigned getChildNumber() const override;

            void apply(NodeVisitor* visitor) override;

            /// \brief Returns whether the \p node is an instance of this class.
            static bool ClassOf(const NodeRef node);
            /// \brief Creates a new instance of this node.
            static NodeRef Create(NodeRef qaNode);
    };

    /// \brief NDQOp specialized for barrier operation.
    class NDQOpBarrier : public NDQOp {
        private:
            enum ChildType {
                I_ONLY = 0
            };

            NDQOpBarrier(NodeRef qaNode);

        public:
            /// \brief Gets the quantum arguments.
            NDList* getQArgs() const;

            Kind getKind() const override;

            std::string getOperation() const override;
            std::string toString(bool pretty = false) const override;

            unsigned getChildNumber() const override;

            void apply(NodeVisitor* visitor) override;

            /// \brief Returns whether the \p node is an instance of this class.
            static bool ClassOf(const NodeRef node);
            /// \brief Creates a new instance of this node.
            static NodeRef Create(NodeRef qaNode);
    };

    /// \brief NDQOp specialized for barrier operation.
    class NDQOpCX : public NDQOp {
        private:
            enum ChildType {
                I_LHS = 0,
                I_RHS
            };

            NDQOpCX(NodeRef lhsNode, NodeRef rhsNode);

        public:
            /// \brief Gets the left hand side of the gate.
            NodeRef getLhs() const;
            /// \brief Gets the right hand side of the gate.
            NodeRef getRhs() const;

            Kind getKind() const override;

            std::string getOperation() const override;
            std::string toString(bool pretty = false) const override;

            unsigned getChildNumber() const override;

            void apply(NodeVisitor* visitor) override;

            /// \brief Returns whether the \p node is an instance of this class.
            static bool ClassOf(const NodeRef node);
            /// \brief Creates a new instance of this node.
            static NodeRef Create(NodeRef lhsNode, NodeRef rhsNode);
    };

    /// \brief NDQOp specialized for barrier operation.
    class NDQOpU : public NDQOp {
        private:
            enum ChildType {
                I_ARGS = 0,
                I_QARG
            };

            NDQOpU(NodeRef aNode, NodeRef qaNode);

        public:
            /// \brief Gets the arguments.
            NodeRef getArgs() const;
            /// \brief Gets the quantum argument.
            NodeRef getQArg() const;

            Kind getKind() const override;

            std::string getOperation() const override;
            std::string toString(bool pretty = false) const override;

            unsigned getChildNumber() const override;

            void apply(NodeVisitor* visitor) override;

            /// \brief Returns whether the \p node is an instance of this class.
            static bool ClassOf(const NodeRef node);
            /// \brief Creates a new instance of this node.
            static NodeRef Create(NodeRef aNode, NodeRef qaNode);
    };

    /// \brief NDQOp specialized for generic operation.
    class NDQOpGeneric : public NDQOp {
        private:
            enum ChildType {
                I_ID = 0,
                I_ARGS,
                I_QARGS
            };

            NDQOpGeneric(NodeRef idNode, NodeRef aNode, NodeRef qaNode);

        public:
            /// \brief Gets the id.
            NDId* getId() const;
            /// \brief Gets the arguments.
            NDList* getArgs() const;
            /// \brief Gets the quantum arguments.
            NDList* getQArgs() const;

            Kind getKind() const override;

            std::string getOperation() const override;
            std::string toString(bool pretty = false) const override;

            unsigned getChildNumber() const override;
            
            void apply(NodeVisitor* visitor) override;

            /// \brief Returns whether the \p node is an instance of this class.
            static bool ClassOf(const NodeRef node);
            /// \brief Creates a new instance of this node.
            static NodeRef Create(NodeRef idNode, NodeRef aNode, NodeRef qaNode);
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

            NDBinOp(OpType t, NodeRef lhsNode, NodeRef rhsNode);

        public:
            /// \brief Gets the left hand side argument.
            NodeRef getLhs() const;
            /// \brief Gets the right hand side argument.
            NodeRef getRhs() const;

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

            unsigned getChildNumber() const override;

            void apply(NodeVisitor* visitor) override;

            /// \brief Returns whether the \p node is an instance of this class.
            static bool ClassOf(const NodeRef node);
            /// \brief Creates a new instance of this node.
            static NodeRef Create(OpType t, NodeRef lhsNode, NodeRef rhsNode);
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

            NDUnaryOp(UOpType t, NodeRef oNode);

        public:
            /// \brief Gets the only operand.
            NodeRef getOperand() const;

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

            unsigned getChildNumber() const override;

            void apply(NodeVisitor* visitor) override;

            /// \brief Returns whether the \p node is an instance of this class.
            static bool ClassOf(const NodeRef node);
            /// \brief Creates a new instance of this node.
            static NodeRef Create(UOpType t, NodeRef oNode);
    };

    /// \brief Node for id references (register specific positions).
    class NDIdRef : public Node {
        private:
            enum ChildType {
                I_ID = 0,
                I_N
            };

            NDIdRef(NodeRef idNode, NodeRef nNode);

        public:
            /// \brief Gets the id.
            NDId* getId() const;
            /// \brief Gets an integer representing the position.
            NDInt* getN() const;

            Kind getKind() const override;

            std::string toString(bool pretty = false) const override;

            unsigned getChildNumber() const override;

            void apply(NodeVisitor* visitor) override;

            /// \brief Returns whether the \p node is an instance of this class.
            static bool ClassOf(const NodeRef node);
            /// \brief Creates a new instance of this node.
            static NodeRef Create(NodeRef idNode, NodeRef nNode);
    };

    /// \brief Base class for list of nodes.
    class NDList : public Node {
        private:
            NDList();

        protected:
            NDList(Kind k, unsigned size);

        public:
            /// \brief Gets the i-th child.
            NodeRef getChild(unsigned i) const;

            Kind getKind() const override;

            std::string toString(bool pretty = false) const override;

            unsigned getChildNumber() const override;

            void apply(NodeVisitor* visitor) override;

            /// \brief Appends a child to the end of the list.
            void addChild(NodeRef child);

            /// \brief Returns whether the \p node is an instance of this class.
            static bool ClassOf(const NodeRef node);
            /// \brief Creates a new instance of this node.
            static NodeRef Create();
    };

    /// \brief Node for list of qubit operation sequences.
    class NDStmtList : public NDList {
        private:
            NDStmtList();

        public:
            Kind getKind() const override;

            std::string toString(bool pretty = false) const override;

            void apply(NodeVisitor* visitor) override;

            /// \brief Returns whether the \p node is an instance of this class.
            static bool ClassOf(const NodeRef node);
            /// \brief Creates a new instance of this node.
            static NodeRef Create();
    };

    /// \brief Node for list of qubit operation sequences inside gate declarations.
    class NDGOpList : public NDList {
        private:
            NDGOpList();

        public:
            Kind getKind() const override;

            std::string toString(bool pretty = false) const override;

            void apply(NodeVisitor* visitor) override;

            /// \brief Returns whether the \p node is an instance of this class.
            static bool ClassOf(const NodeRef node);
            /// \brief Creates a new instance of this node.
            static NodeRef Create();
    };

    /// \brief Node for conditional statement.
    class NDIfStmt : public Node {
         public:
            enum ChildType {
                I_COND_ID = 0,
                I_COND_N,
                I_QOP
            };

        private:
            NDIfStmt(NodeRef cidNode, NodeRef nNode, NodeRef qopNode);

        public:
            /// \brief Gets the id inside the conditional.
            NDId* getCondId() const;
            /// \brief Gets the int inside the conditional.
            NDInt* getCondN() const;
            /// \brief Gets the qop.
            NodeRef getQOp() const;

            Kind getKind() const override;

            std::string toString(bool pretty = false) const override;
            std::string getOperation() const override;

            unsigned getChildNumber() const override;

            void apply(NodeVisitor* visitor) override;

            /// \brief Returns whether the \p node is an instance of this class.
            static bool ClassOf(const NodeRef node);
            /// \brief Creates a new instance of this node.
            static NodeRef Create(NodeRef cidNode, NodeRef nNode, NodeRef qopNode);
    };
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
unsigned efd::NDValue<T>::getChildNumber() const {
    return 0;
}

template <typename T>
efd::NodeRef efd::NDValue<T>::Create(T val) {
    return new NDValue<T>(val);
}

#endif
