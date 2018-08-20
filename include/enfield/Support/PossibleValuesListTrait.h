#ifndef __EFD_POSSIBLE_VALUES_LIST_TRAIT_H__
#define __EFD_POSSIBLE_VALUES_LIST_TRAIT_H__

#include "enfield/Support/EnumString.h"

namespace efd {
    template <typename T>
        struct PossibleValuesListTrait {
            static std::vector<std::string> Get() { return {}; }
        };

    template <typename T, T first, T last, uint32_t padding>
        struct PossibleValuesListTrait<efd::EnumString<T, first, last, padding>> {
            static std::vector<std::string> Get() {
                return EnumString<T, first, last, padding>::StringList();
            }
        };
}

#endif
