#ifndef __EFD_NODES_H__
#define __EFD_NODES_H__

#include <iostream>
#include <vector>
#include <memory>

namespace efd {
    class Node {
        public:
            typedef std::vector< std::shared_ptr<Node> >::iterator iterator;
            typedef std::vector< std::shared_ptr<Node> >::const_iterator const_iterator;

            std::vector< std::shared_ptr<Node> > mChild;

            iterator begin();
            iterator end();

            const_iterator begin() const;
            const_iterator end() const;

            void print(std::ostream& O = std::cout, bool endl = false);
            void print(bool endl = false);

            virtual std::string getOperation() const;
            virtual std::string toString(bool endl = false) const;
    };

    class NDDecl : public Node {
        public:
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
            NDDecl(Type t);

            bool isCReg() const;
            bool isQReg() const;

            std::string getOperation() const override;
            std::string toString(bool endl) const override;
    };

    class NDGateDecl : public Node {
        private:
            enum ChildType {
                I_ID = 0,
                I_ARGS,
                I_QARGS,
                I_GOPLIST
            };

        public:
            NDGateDecl();
            std::string getOperation() const override;
            std::string toString(bool endl) const override;
    };

    class NDGOpList : public Node {
        public:
            NDGOpList();
            std::string toString(bool endl) const override;
    };

    class NDOpaque : public Node {
        private:
            enum ChildType {
                I_ID = 0,
                I_ARGS,
                I_QARGS
            };

        public:
            NDOpaque();
            std::string getOperation() const override;
            std::string toString(bool endl) const override;
    };

    class NDQOpMeasure : public Node {
        private:
            enum ChildType {
                I_QBIT = 0,
                I_CBIT
            };

        public:
            NDQOpMeasure();
            std::string getOperation() const override;
            std::string toString(bool endl) const override;
    };

    class NDQOpReset : public Node {
        public:
            NDQOpReset();
            std::string getOperation() const override;
    };

    class NDQOpBarrier : public Node {
        public:
            NDQOpBarrier();
            std::string getOperation() const override;
    };

    class NDQOpCall : public Node {
        private:
            enum ChildType {
                I_QBIT = 0,
                I_CBIT
            };

        public:
            NDQOpCall();
            std::string getOperation() const override;
            std::string toString(bool endl) const override;
    };
};

#endif
