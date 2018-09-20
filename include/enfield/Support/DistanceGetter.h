#ifndef __EFD_DISTANCE_GETTER_H__
#define __EFD_DISTANCE_GETTER_H__

#include "enfield/Support/Graph.h"

namespace efd {
    /// \brief Interface for calculating the distance of the path between two vertices.
    template <typename T>
        class DistanceGetter {
            public:
                typedef DistanceGetter* Ref;
                typedef std::shared_ptr<DistanceGetter> sRef;
                typedef std::unique_ptr<DistanceGetter> uRef;

            protected:
                Graph::Ref mG;

                virtual void initImpl();
                virtual T getImpl(uint32_t u, uint32_t v) = 0;

            private:
                void checkInitialized();
                void checkVertexInGraph(uint32_t u);

            public:
                DistanceGetter();
                virtual ~DistanceGetter() = default;

                void init(Graph::Ref graph);

                T get(uint32_t u, uint32_t v);
        };
}

template <typename T>
efd::DistanceGetter<T>::DistanceGetter() : mG(nullptr) {}

template <typename T>
void efd::DistanceGetter<T>::initImpl() {}

template <typename T>
void efd::DistanceGetter<T>::checkInitialized() {
    if (mG == nullptr) {
        ERR << "Set `Graph` for the DistanceGetter!" << std::endl;
        EFD_ABORT();
    }
}

template <typename T>
void efd::DistanceGetter<T>::checkVertexInGraph(uint32_t u) {
    if (u >= mG->size()) {
        ERR << "Out of Bounds: can't calculate distance for: `" << u << "`" << std::endl;
        EFD_ABORT();
    }
}

template <typename T>
void efd::DistanceGetter<T>::init(Graph::Ref graph) {
    if (graph != nullptr) {
        mG = graph;
        initImpl();
    }
}

template <typename T>
T efd::DistanceGetter<T>::get(uint32_t u, uint32_t v) {
    checkInitialized();
    checkVertexInGraph(u);
    checkVertexInGraph(v);
    return getImpl(u, v);
}

#endif
