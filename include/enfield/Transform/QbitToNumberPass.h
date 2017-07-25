#ifndef __EFD_QBIT_TO_NUMBER_PASS_H__
#define __EFD_QBIT_TO_NUMBER_PASS_H__

#include "enfield/Transform/Pass.h"
#include "enfield/Transform/QModule.h"
#include "enfield/Analysis/Nodes.h"

#include <unordered_map>
#include <vector>
#include <set>

namespace efd {

    /// \brief Maps every quantum bit (not register) to a number inside a vector.
    ///
    /// It also keeps track of the qbits inside every gate declaration. Inside every
    /// scope, it maps the qbits existing inside them to an unsigned number.
    /// 
    /// Note that if "qreg r[10];" declaration exists, then "r" is not a qbit, but
    /// "r[n]" is (where "n" is in "{0 .. 9}").
    struct QbitToNumber {
        struct QbitInfo {
            std::string mKey;
            Node::sRef mRef;
        };

        typedef std::vector<QbitInfo> QbitMap;

        std::unordered_map<NDGateDecl*, QbitMap> mLIdMap;
        QbitMap mGIdMap;

        QbitToNumber();

        /// \brief Gets a QbitInfo map for the specified gate.
        const QbitMap* getMap(NDGateDecl::Ref gate) const;

        /// \brief Returns an unsigned number representing the id
        /// in this specific gate (if any).
        unsigned getUId(std::string id, NDGateDecl::Ref gate = nullptr) const;

        /// \brief Returns the number of qbits in a given gate (if any).
        unsigned getSize(NDGateDecl::Ref gate = nullptr) const;

        /// \brief Returns the std::string id representation of the
        /// corresponding unsigned id in the specific gate (if any).
        std::string getStrId(unsigned id, NDGateDecl::Ref gate = nullptr) const;

        /// \brief Get a Node::Ref, representing that qbit.
        Node::Ref getNode(unsigned id, NDGateDecl::Ref gate = nullptr) const;
    };

    /// \brief WrapperPass that yields a \em QbitToNumber structure.
    class QbitToNumberWrapperPass : public PassT<QbitToNumber> {
        public:
            typedef QbitToNumberWrapperPass* Ref;
            typedef std::unique_ptr<QbitToNumberWrapperPass> uRef;
            typedef std::shared_ptr<QbitToNumberWrapperPass> sRef;

            void run(QModule::Ref qmod) override;

            /// \brief Returns a new instance of this class.
            static uRef Create();
    };
};

#endif
