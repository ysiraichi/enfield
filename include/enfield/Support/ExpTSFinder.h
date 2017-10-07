#ifndef __EFD_EXP_TS_FINDER_H__
#define __EFD_EXP_TS_FINDER_H__

#include "enfield/Support/TokenSwapFinder.h"
#include <map>

namespace efd {
    class ExpTSFinder : public TokenSwapFinder {
        public:
            typedef std::unique_ptr<ExpTSFinder> uRef;

        private:
            std::map<Assign, uint32_t> mMapId;
            std::vector<std::vector<Swap>> mSwaps;

            void genAllAssigns(uint32_t n);
            void preprocess(Graph::Ref graph);
            uint32_t getTargetId(Assign source, Assign target);

        public:
            std::vector<Assign> mAssigns;

            ExpTSFinder(Graph::sRef graph);

            std::vector<Swap> find(Assign from, Assign to) override;
            std::vector<Swap> find(Graph::Ref graph, Assign from, Assign to) override;

            /// \brief Creates an instance of this class.
            uRef Create(Graph::sRef graph = nullptr);
    };
}

#endif
