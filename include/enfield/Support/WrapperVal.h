#ifndef __EFD_WRAPPER_VAL_H__
#define __EFD_WRAPPER_VAL_H__

#include <string>

namespace efd {

    /// \brief Wrapper for primitive values
    ///
    /// They are used mainly for keeping the right string representation of the
    /// primitive values in the program.
    template <typename T>
    struct WrapperVal {
        /// \brief The static representation.
        T mV;
        /// \brief The string representation of the double.
        std::string mStr;

        WrapperVal();
        /// \brief Parses the string to a double value.
        WrapperVal(std::string str);
    };

    template class WrapperVal<long long>;
    template class WrapperVal<double>;

    template <> WrapperVal<long long>::WrapperVal(std::string str);
    template <> WrapperVal<double>::WrapperVal(std::string str);

    typedef WrapperVal<long long> IntVal;
    typedef WrapperVal<double> RealVal;

};

template <typename T>
efd::WrapperVal<T>::WrapperVal() : mStr("") {}

namespace std {
    /// \brief Overloading std::to_string to work with efd::WrapperVal.
    template <typename T>
        string to_string(const efd::WrapperVal<T>& val) {
            return val.mStr;
        }
};

#endif
