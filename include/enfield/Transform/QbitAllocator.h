#ifndef __EFD_QBIT_ALLOCATOR_H__
#define __EFD_QBIT_ALLOCATOR_H__

#include "enfield/Support/Graph.h"
#include "enfield/Support/SwapFinding.h"
#include "enfield/Transform/DependencyBuilderPass.h"

namespace efd {
    /// \brief Base abstract class that allocates the qbits used in the program to
    /// the qbits that are in the physical architecture.
    class QbitAllocator {
        public:
            typedef std::vector<unsigned> Mapping;
            typedef efd::DependencyBuilderPass::DepsSet DepsSet;
            typedef DepsSet::iterator Iterator;

        private:
            DependencyBuilderPass* mDepPass;
            DepsSet mDepSet;
            Mapping mMapping;
            bool mRun;

            /// \brief Updates the mDepSet attribute. Generally it is done after
            /// running the DependencyBuilderPass.
            void updateDepSet();

        protected:
            QModule* mMod;
            Graph* mPhysGraph;
            SwapFinding* mSFind;

            QbitAllocator(QModule* qmod, Graph* pGraph, SwapFinding* sFind,
                    DependencyBuilderPass* depPass);

            /// \brief Inlines the gate call that generates the dependencies that are
            /// referenced by \p it. If the node is not an NDQOpGeneric, it does nothing.
            Iterator inlineDep(Iterator it);

        public:
            /// \brief Runs the allocator;
            void run();
            /// \brief Returns the final mapping.
            Mapping getMapping();

            /// \brief Generates the final mapping of the program.
            virtual Mapping generateMapping(DepsSet& deps) = 0;
    };
}

#endif
