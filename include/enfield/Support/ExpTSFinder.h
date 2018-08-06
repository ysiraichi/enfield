#ifndef __EFD_EXP_TS_FINDER_H__
#define __EFD_EXP_TS_FINDER_H__

#include "enfield/Support/TokenSwapFinder.h"
#include <map>

namespace efd {
    /// \brief Brute force solution to the Token Swap Finder.
    ///
    /// It first preprocesses the given graph, generating all permutations
    /// from one starting point. This is the expensive part of the process.
    /// It takes O(|Q|!).
    ///
    /// At each query, it renames the qubits and finds the swaps needed to
    /// solve that query.
    /// Each query only takes O(|Q|).
    class ExpTSFinder : public TokenSwapFinder {
        public:
            typedef std::unique_ptr<ExpTSFinder> uRef;

        private:
            std::map<InverseMap, uint32_t> mMapId;
            std::vector<SwapSeq> mSwaps;

            void genAllAssigns(uint32_t n);
            uint32_t getTargetId(const InverseMap& source, const InverseMap& target);

            void preprocess() override;
            SwapSeq findImpl(const InverseMap& from, const InverseMap& to) override;

        public:
            std::vector<InverseMap> mInverseMaps;

            /// \brief Creates an instance of this class.
            static uRef Create();
    };
}

#endif
