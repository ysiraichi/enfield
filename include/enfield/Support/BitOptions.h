#ifndef __EFD_BIT_OPTIONS_H__
#define __EFD_BIT_OPTIONS_H__

#include "enfield/Support/Defs.h"
#include <cstring>

namespace efd {
    /// \brief Implements bit options for size last.
    template <typename ETy, ETy last>
        class BitOptions {
            private:
                // Size of the representation in bytes * Number of bits for each byte
                constexpr static uint32_t Last = static_cast<uint32_t>(last);
                constexpr static uint32_t ElemSize = sizeof(uint8_t) * 8;
                constexpr static uint32_t TotalSize = ((Last) / ElemSize) + 1;

                static_assert(Last >= 0, "Cannot create BinOption with negative options.");

                uint8_t mV[TotalSize];

                /// \brief Gets the block and bit from the \p option.
                inline std::pair<uint32_t, uint32_t> getBlkAndBit(ETy option) const;

            public:
                BitOptions();

                /// \brief Sets the bit related to \p option to \p value.
                void set(ETy option, bool value = true);
                /// \brief Gets the value related to the bit that \p option represents.
                bool get(ETy option) const;
        };
}

template <typename ETy, ETy last>
efd::BitOptions<ETy, last>::BitOptions() {
    memset(mV, 0, TotalSize);
}

template <typename ETy, ETy last>
std::pair<uint32_t, uint32_t> efd::BitOptions<ETy, last>::getBlkAndBit(ETy option) const {
    uint32_t iOpt = static_cast<uint32_t>(option);
    EfdAbortIf(iOpt > Last, "Invalid Option: " << iOpt << ". Max: " << Last);

    uint32_t blk = iOpt / ElemSize;
    uint32_t bit = iOpt % ElemSize;
    return std::make_pair(blk, bit);
}

template <typename ETy, ETy last>
void efd::BitOptions<ETy, last>::set(ETy option, bool value) {
    auto pair = getBlkAndBit(option);
    auto blkValue = mV[pair.first];
    auto iVal = static_cast<uint32_t>(value);
    mV[pair.first] = (blkValue - (blkValue & (1 << pair.second))) + (iVal << pair.second);
}

template <typename ETy, ETy last>
bool efd::BitOptions<ETy, last>::get(ETy option) const {
    auto pair = getBlkAndBit(option);
    return static_cast<bool>(mV[pair.first] & (1 << pair.second));
}

#endif
