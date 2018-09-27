#ifndef __EFD_ENUM_STRING_H__
#define __EFD_ENUM_STRING_H__

#include "enfield/Support/Defs.h"

#include <vector>
#include <algorithm>
#include <typeinfo>

namespace efd {
    /// \brief Enum wrapper class.
    ///
    /// Turns Enum to String and vice-versa.
    /// It is used mainly for the command line options.
    template <typename T, T first, T last, uint32_t padding = 0>
        class EnumString {
            private:
                uint32_t mValue;

                /// \brief Initializes this class from an initialization string.
                void initImpl(std::string init);

            protected:
                typedef EnumString<T, first, last, padding> Self;
                static std::vector<std::string> mStrVal;

            public:
                EnumString(T init);
                EnumString(const char* cstr);
                EnumString(std::string init);

                /// \brief Returns the string value of the enum.
                std::string getStringValue() const;
                /// \brief Returns the value of the enum.
                T getValue() const;

                bool operator<(const Self& rhs) const;

                /// \brief Returns a list with every enumerated element in its
                /// string representation.
                static std::vector<std::string> StringList() {
                    if (padding != 0) {
                        return std::vector<std::string>(mStrVal.begin() + padding,
                                                        mStrVal.end() - padding);
                    } else {
                        return mStrVal;
                    }
                }

                /// \brief Returns a list with every enumerated element.
                static std::vector<Self> List() {
                    std::vector<Self> list;

                    for (const std::string& str : StringList())
                        list.push_back(Self(str));

                    return list;
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
    template <typename T, T first, T last, uint32_t padding>
        string to_string(efd::EnumString<T, first, last, padding>& val) {
            return val.getStringValue();
        }
}

template <typename T, T first, T last, uint32_t padding>
efd::EnumString<T, first, last, padding>::EnumString(T init) {
    mValue = static_cast<uint32_t>(init);

    EfdAbortIf(mValue > static_cast<uint32_t>(last) || mValue < static_cast<uint32_t>(first),
               "Enum '" << typeid(T).name() << "' doesn't have such value '" << mValue << "'.");

    mValue = mValue - static_cast<uint32_t>(first);
}

template <typename T, T first, T last, uint32_t padding>
efd::EnumString<T, first, last, padding>::EnumString(const char* cstr) {
    initImpl(std::string(cstr));
}

template <typename T, T first, T last, uint32_t padding>
efd::EnumString<T, first, last, padding>::EnumString(std::string init) {
    initImpl(init);
}

template <typename T, T first, T last, uint32_t padding>
void efd::EnumString<T, first, last, padding>::initImpl(std::string init) {
    EfdAbortIf((!EnumString<T, first, last, padding>::Has(init)),
               "Enum '" << typeid(T).name() << "' doesn't have string '" << init << "'.");

    auto it = std::find(mStrVal.begin(), mStrVal.end(), init);
    mValue = std::distance(mStrVal.begin(), it);
}

template <typename T, T first, T last, uint32_t padding>
std::string efd::EnumString<T, first, last, padding>::getStringValue() const {
    auto strSize = mStrVal.size();

    EfdAbortIf(strSize < mValue,
               "Enum '" << typeid(T).name() << "' does not have value over "
               << strSize << ": " << mValue);

    if (static_cast<uint32_t>(last) - static_cast<uint32_t>(first) + 1 != strSize) {
        WAR << "Enum '" << typeid(T).name() << "' segmented. "
            << "Returning wrong string." << std::endl;
    }

    return mStrVal[mValue];
}

template <typename T, T first, T last, uint32_t padding>
T efd::EnumString<T, first, last, padding>::getValue() const {
    return static_cast<T>(mValue + static_cast<uint32_t>(first));
}

template <typename T, T first, T last, uint32_t padding>
bool efd::EnumString<T, first, last, padding>::operator<(const Self& rhs) const {
    return mValue < rhs.mValue;
}

#endif 
