#ifndef __EFD_SIMPLE_QALLOCATOR_H__
#define __EFD_SIMPLE_QALLOCATOR_H__

#include "enfield/Transform/Allocators/StdSolutionQAllocator.h"
#include "enfield/Support/BitOptions.h"

namespace efd {
    /// \brief Interface for finding a mapping from some set of dependencies.
    class MappingFinder {
        public:
            typedef QbitAllocator::DepsVector DepsVector;

            typedef MappingFinder* Ref;
            typedef std::shared_ptr<MappingFinder> sRef;

            virtual ~MappingFinder() = default;

            /// \brief Returns a mapping generated from a set of dependencies.
            virtual Mapping find(ArchGraph::Ref g, DepsVector& deps) = 0;
    };

    /// \brief Options for solution builders.
    enum class SolutionBuilderOptions {
        ImproveInitial = 0,
        KeepStats
    };

    /// \brief Interface for building the solution from an initial mapping.
    class SolutionBuilder : public BitOptions<SolutionBuilderOptions,
                                              SolutionBuilderOptions::KeepStats> {
        public:
            typedef QbitAllocator::DepsVector DepsVector;

            typedef SolutionBuilder* Ref;
            typedef std::shared_ptr<SolutionBuilder> sRef;

        public:
            virtual ~SolutionBuilder() = default;

            SolutionBuilder() {
                set(SolutionBuilderOptions::ImproveInitial);
                set(SolutionBuilderOptions::KeepStats);
            }

            /// \brief Constructs a solution (\em QbitAllocator::Solution) from the
            /// mapping \p initial, with \p deps dependencies in the architecture \p g.
            virtual StdSolution build(Mapping initial, DepsVector& deps, ArchGraph::Ref g) = 0;
    };

    /// \brief DependencySolver that allocates the logical qubits from an initial mapping.
    class SimpleQAllocator : public StdSolutionQAllocator {
        public:
            typedef SimpleQAllocator* Ref;
            typedef std::unique_ptr<SimpleQAllocator> uRef;

        protected:
            MappingFinder::sRef mMapFinder;
            SolutionBuilder::sRef mSolBuilder;

            SimpleQAllocator(ArchGraph::sRef agraph);

            StdSolution buildStdSolution(QModule::Ref qmod) override;

        public:
            /// \brief Sets the mapping finder to \p finder.
            void setMapFinder(MappingFinder::sRef finder);
            /// \brief Sets the solution builder to \p builder.
            void setSolBuilder(SolutionBuilder::sRef builder);

            /// \brief Creates an instance of this class.
            static uRef Create(ArchGraph::sRef agraph);
    };
}

#endif
