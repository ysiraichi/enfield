#ifndef __EFD_APPROX_TS_FINDER_H__
#define __EFD_APPROX_TS_FINDER_H__

#include "enfield/Support/TokenSwapFinder.h"

namespace efd {
    class ApproxTSFinder : public TokenSwapFinder {
        public:
            typedef ApproxTSFinder* Ref;
            typedef std::unique_ptr<ApproxTSFinder> uRef;

            ApproxTSFinder(Graph::sRef graph);

            SwapSeq find(Assign from, Assign to) override;
            SwapSeq find(Graph::Ref graph, Assign from, Assign to) override;

            /// \brief Creates an instance of this class.
            uRef Create(Graph::sRef graph = nullptr);
    };
}

#endif
