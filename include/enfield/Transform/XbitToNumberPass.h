#ifndef __EFD_XBIT_TO_NUMBER_PASS_H__
#define __EFD_XBIT_TO_NUMBER_PASS_H__

#include "enfield/Transform/Pass.h"
#include "enfield/Transform/QModule.h"
#include "enfield/Analysis/Nodes.h"

#include <unordered_map>
#include <vector>
#include <set>
#include <map>

namespace efd {

    /// \brief Maps every quantum and classic bit (not register) to a number inside
    /// a vector.
    ///
    /// It also keeps track of the qubits inside every gate declaration. Inside every
    /// scope, it maps the qubits existing inside them to an uint32_t number.
    /// 
    /// Note that if "qreg r[10];" declaration exists, then "r" is not a qbit, but
    /// "r[n]" is (where "n" is in "{0 .. 9}").
    struct XbitToNumber {
        struct XbitInfo {
            uint32_t key;
            Node::sRef node;
        };

        typedef std::map<std::string, XbitInfo> XbitMap;
        typedef std::map<std::string, std::vector<uint32_t>> XRegMap;

        std::unordered_map<NDGateDecl*, XbitMap> lidQMap;
        XbitMap gidQMap;
        XbitMap gidCMap;
        XRegMap gidRegMap;

        /// \brief Gets a constant reference to the mapping of qubtis of a gate.
        const XbitMap& getQbitMap(NDGateDecl::Ref gate = nullptr) const;

        /// \brief Returns a list of uids that relate to a given register.
        std::vector<uint32_t> getRegUIds(std::string id) const;

        /// \brief Returns an uint32_t number representing the qubit
        /// in this specific gate (if any).
        uint32_t getQUId(std::string id, NDGateDecl::Ref gate = nullptr) const;
        /// \brief Returns an uint32_t number representing the classic bit;
        uint32_t getCUId(std::string id) const;

        /// \brief Returns the number of qbits in a given gate (if any).
        uint32_t getQSize(NDGateDecl::Ref gate = nullptr) const;
        /// \brief Returns the number of cbits in a given gate (if any).
        uint32_t getCSize() const;

        /// \brief Returns the std::string id representation of the
        /// corresponding qubit, represented by uint32_t id in the
        /// specific gate (if any).
        std::string getQStrId(uint32_t id, NDGateDecl::Ref gate = nullptr) const;
        /// \brief Returns the std::string id representation of the
        /// corresponding classic bit, represented by id.
        std::string getCStrId(uint32_t id) const;

        /// \brief Get a Node::Ref, representing that qbit.
        Node::Ref getQNode(uint32_t id, NDGateDecl::Ref gate = nullptr) const;
        /// \brief Get a Node::Ref, representing that cbit.
        Node::Ref getCNode(uint32_t id) const;
    };

    /// \brief WrapperPass that yields a \em XbitToNumber structure.
    class XbitToNumberWrapperPass : public PassT<XbitToNumber> {
        public:
            typedef XbitToNumberWrapperPass* Ref;
            typedef std::unique_ptr<XbitToNumberWrapperPass> uRef;
            typedef std::shared_ptr<XbitToNumberWrapperPass> sRef;

            static uint8_t ID;

            bool run(QModule::Ref qmod) override;

            /// \brief Returns a new instance of this class.
            static uRef Create();
    };
};

#endif
