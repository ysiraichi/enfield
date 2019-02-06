#ifndef __EFD_IBM_QALLOCATOR_H__
#define __EFD_IBM_QALLOCATOR_H__

#include "enfield/Transform/Allocators/StdSolutionQAllocator.h"
#include "enfield/Transform/LayersBuilderPass.h"
#include "enfield/Support/Defs.h"

namespace efd {
    /// \brief Port of IBM's allocator.
    ///
    /// Main python implementation can be found at:
    /// https://github.com/Qiskit/qiskit-terra/blob/master/qiskit/mapper/_mapping.py
    class IBMQAllocator : public StdSolutionQAllocator {
        public:
            typedef IBMQAllocator* Ref;
            typedef std::unique_ptr<IBMQAllocator> uRef;

        private:
            struct AllocationResult {
                Mapping map;
                bool success;
                StdSolution::OpVector opv;
                bool isTrivialLayer;
            };

            uint32_t mPQubits;
            uint32_t mLQubits;
            std::vector<std::vector<uint32_t>> mDist;

            AllocationResult tryAllocateLayer(Layer& layer, Mapping current,
                                              std::set<uint32_t> qubitsSet,
                                              DependencyBuilder& depData);

        public:
            IBMQAllocator(ArchGraph::sRef archGraph);

            StdSolution buildStdSolution(QModule::Ref qmod) override;
            static uRef Create(ArchGraph::sRef archGraph);
    };
}

#endif
