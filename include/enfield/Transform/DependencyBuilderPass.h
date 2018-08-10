#ifndef __EFD_DEPENDENCY_BUILDER_PASS_H__
#define __EFD_DEPENDENCY_BUILDER_PASS_H__

#include "enfield/Analysis/Nodes.h"
#include "enfield/Transform/Pass.h"
#include "enfield/Transform/QModule.h"
#include "enfield/Transform/XbitToNumberPass.h"

#include <unordered_map>
#include <map>
#include <vector>
#include <set>

namespace efd {

    /// \brief Structure for abstracting dependencies.
    struct Dep {
        uint32_t mFrom;
        uint32_t mTo;
    };

    /// \brief Represents a sequence of dependencies (should be treated as
    /// parallel dependencies) for each node.
    struct Dependencies {
        typedef std::vector<Dep>::iterator Iterator;
        typedef std::vector<Dep>::const_iterator ConstIterator;

        std::vector<Dep> mDeps;
        Node::Ref mCallPoint;

        /// \brief Forwards to the \em mDeps attribute.
        const Dep& operator[](uint32_t i) const;
        /// \brief Forwards to the \em mDeps attribute.
        Dep& operator[](uint32_t i);

        /// \brief Forwards to the \em mDeps attribute.
        bool empty() const;

        /// \brief Forwards to the \em mDeps attribute.
        uint32_t size() const;

        /// \brief Forwards to the \em mDeps attribute.
        Iterator begin();
        /// \brief Forwards to the \em mDeps attribute.
        ConstIterator begin() const;
        /// \brief Forwards to the \em mDeps attribute.
        Iterator end();
        /// \brief Forwards to the \em mDeps attribute.
        ConstIterator end() const;
    };

    /// \brief Keep track of the dependencies of each qbit for the whole program,
    /// as well as the dependencies for every gate.
    ///
    /// Each gate, as well as the whole program have one 'DepsVector' variable. The idea is
    /// to store a sequence of parallel dependencies. Here, parallel dependency is a
    /// dependency that can't be broken down (unless the gate is inlined).
    struct DependencyBuilder {
        typedef DependencyBuilder* Ref;
        typedef std::vector<Dependencies> DepsVector;

        XbitToNumber mXbitToNumber;

        std::unordered_map<NDGateDecl*, DepsVector> mLDeps;
        std::map<Node*, Dependencies> mIDeps;
        DepsVector mGDeps;

        DependencyBuilder();

        /// \brief Gets an unsingned id for \p ref.
        uint32_t getUId(Node::Ref ref, NDGateDecl::Ref gate);
        /// \brief Gets the DepsVector corresponding to the current quantum gate,
        /// or the global (if current gate is null).
        const DepsVector* getDepsVector(NDGateDecl::Ref gate = nullptr) const;
        DepsVector* getDepsVector(NDGateDecl::Ref gate = nullptr);

        /// \brief Returns the structure that mapped the qbits.
        XbitToNumber& getXbitToNumber();
        /// \brief Sets the structure that will map the qbits.
        void setXbitToNumber(XbitToNumber& xtn);

        /// \brief Gets the dependencies for some gate declaration. If it is a
        /// nullptr, then it is returned the dependencies for the whole program.
        const DepsVector& getDependencies(NDGateDecl::Ref ref = nullptr) const;
        DepsVector& getDependencies(NDGateDecl::Ref ref = nullptr);

        /// \brief Gets the dependencies for a specific instruction.
        const Dependencies getDeps(Node* ref) const;
        Dependencies getDeps(Node* ref);
    };

    /// \brief WrapperPass that yields a \em DependencyBuilder structure.
    class DependencyBuilderWrapperPass : public PassT<DependencyBuilder> {
        public:
            typedef DependencyBuilderWrapperPass* Ref;
            typedef std::unique_ptr<DependencyBuilderWrapperPass> uRef;
            typedef std::shared_ptr<DependencyBuilderWrapperPass> sRef;

            static uint8_t ID;

            bool run(QModule::Ref qmod) override;

            /// \brief Returns a new instance of this class.
            static uRef Create();
    };
};

#endif
