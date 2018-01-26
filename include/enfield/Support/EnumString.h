#ifndef __EFD_ENUM_STRING_H__
#define __EFD_ENUM_STRING_H__

#include "enfield/Support/Defs.h"

#include <vector>
#include <algorithm>
#include <typeinfo>
#include <cassert>

namespace efd {
    /// \brief Enum wrapper class.
    ///
    /// Turns Enum to String and vice-versa.
    /// It is used mainly for the command line options.
    template <typename T, T first, T last>
        class EnumString {
            private:
                static std::vector<std::string> mStrVal;
                uint32_t mValue;

                /// \brief Initializes this class from an initialization string.
                void initImpl(std::string init);

            public:
                EnumString(T init);
                EnumString(const char* cstr);
                EnumString(std::string init);

                /// \brief Returns the string value of the enum.
                std::string getStringValue() const;
                /// \brief Returns the value of the enum.
                T getValue() const;

                bool operator<(const EnumString<T, first, last>& rhs) const;

                /// \brief Returns a list with every enumerated element in its
                /// string representation.
                static std::vector<std::string> List() {
                    return mStrVal;
                }

                /// \brief Returns whether \p str is a string representative of any
                /// element of this class.
                static bool Has(std::string str) {
                    auto it = std::find(mStrVal.begin(), mStrVal.end(), str);
                    return it != mStrVal.end();
                }
        };
}

namespace std {
    template <typename T, T first, T last>
        string to_string(efd::EnumString<T, first, last>& val) {
            return val.getStringValue();
        }
}

template <typename T, T first, T last>
efd::EnumString<T, first, last>::EnumString(T init) {
    mValue = static_cast<uint32_t>(init);

    if (mValue > static_cast<uint32_t>(last) || mValue < static_cast<uint32_t>(first)) {
        ERR << "Enum '" << typeid(T).name() << "' doesn't have such value '"
            << mValue << "'." << std::endl;
        assert(false && "Invalid init value.");
    }

    mValue = mValue - static_cast<uint32_t>(first);
}

template <typename T, T first, T last>
efd::EnumString<T, first, last>::EnumString(const char* cstr) {
    initImpl(std::string(cstr));
}

template <typename T, T first, T last>
efd::EnumString<T, first, last>::EnumString(std::string init) {
    initImpl(init);
}

template <typename T, T first, T last>
void efd::EnumString<T, first, last>::initImpl(std::string init) {
    if (!EnumString<T, first, last>::Has(init)) {
        ERR << "Enum '" << typeid(T).name() << "' doesn't have string '"
            << init << "'." << std::endl;
        assert(false && "Invalid init string.");
    }

    auto it = std::find(mStrVal.begin(), mStrVal.end(), init);
    mValue = std::distance(mStrVal.begin(), it);
}

template <typename T, T first, T last>
std::string efd::EnumString<T, first, last>::getStringValue() const {
    auto strSize = mStrVal.size();

    if (strSize < mValue) {
        ERR << "Enum '" << typeid(T).name() << "' does not have value over "
            << strSize << ": " << mValue << std::endl;
        assert(false && "Broken enum value.");
    }

    if (static_cast<uint32_t>(last) - static_cast<uint32_t>(first) + 1 != strSize) {
        WAR << "Enum '" << typeid(T).name() << "' segmented. "
            << "Returning wrong string." << std::endl;
    }

    return mStrVal[mValue];
}

template <typename T, T first, T last>
T efd::EnumString<T, first, last>::getValue() const {
    return static_cast<T>(mValue + static_cast<uint32_t>(first));
}

template <typename T, T first, T last>
bool efd::EnumString<T, first, last>::operator<(const EnumString<T, first, last>& rhs) const {
    return mValue < rhs.mValue;
}

#endif 
