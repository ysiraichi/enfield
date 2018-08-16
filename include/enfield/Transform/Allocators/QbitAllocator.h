#ifndef __EFD_QBIT_ALLOCATOR_H__
#define __EFD_QBIT_ALLOCATOR_H__

#include "enfield/Arch/ArchGraph.h"
#include "enfield/Transform/DependencyBuilderPass.h"
#include "enfield/Transform/Utils.h"
#include "enfield/Support/CommandLine.h"
#include "enfield/Support/Stats.h"

namespace efd {
    /// \brief Base abstract class that allocates the qbits used in the program to
    /// the qbits that are in the physical architecture.
    class QbitAllocator : public PassT<Mapping> {
        public:
            typedef QbitAllocator* Ref;
            typedef std::unique_ptr<QbitAllocator> uRef;

            typedef DependencyBuilder::DepsVector DepsVector;
            typedef DependencyBuilder::DepsVector::iterator Iterator;

        private:
            uint32_t mCXCost;
            uint32_t mHCost;

            /// \brief Calculates the cost of a \em CNOT and a \em H gate, based on the
            /// defined weights.
            void calculateHAndCXCost();

            /// \brief Inlines all gates, but those flagged.
            void inlineAllGates(QModule::Ref qmod);

            /// \brief Replace all qbits from the program with the architecture's qbits. 
            void replaceWithArchSpecs(QModule::Ref qmod);

        protected:
            ArchGraph::sRef mArchGraph;
            GateWeightMap mGateWeightMap;

            uint32_t mVQubits;
            uint32_t mPQubits;

            QbitAllocator(ArchGraph::sRef archGraph);

            /// \brief Executes the allocation algorithm after the preprocessing.
            virtual Mapping allocate(QModule::Ref qmod) = 0;

            /// \brief Returns the cost of a \em CNOT gate, based on the defined weights.
            uint32_t getCXCost(uint32_t u, uint32_t v);
            /// \brief Returns the cost of a \em SWAP gate, based on the defined weights.
            uint32_t getSwapCost(uint32_t u, uint32_t v);
            /// \brief Returns the cost of a \em BRIDGE gate, based on the defined weights.
            uint32_t getBridgeCost(uint32_t u, uint32_t w, uint32_t v);

        public:
            bool run(QModule::Ref qmod) override;

            /// \brief Sets the weights to be used for each gate.
            void setGateWeightMap(const GateWeightMap& weightMap);
    };

    /// \brief Generates an assignment mapping (maps the architecture's qubits
    /// to the logical ones) of size \p archQ.
    InverseMap InvertMapping(uint32_t archQ, Mapping mapping, bool fill = true);

    /// \brief Fills the unmapped qubits with the ones missing.
    void Fill(uint32_t archQ, Mapping& mapping);
    void Fill(Mapping& mapping, InverseMap& inv);

    /// \brief Returns an identity mapping.
    Mapping IdentityMapping(uint32_t progQ);

    /// \brief Prints the mapping \p m to a string and returns it.
    std::string MappingToString(Mapping m);
}

#endif
