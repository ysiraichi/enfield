#ifndef __EFD_STATS_H__
#define __EFD_STATS_H__

#include <iostream>

namespace efd {

    /// \brief Base class for stats.
    class StatBase {
        protected:
            std::string mName;
            std::string mDescription;

        public:
            StatBase(std::string name, std::string description);

            /// \brief Gets the name of the stat.
            std::string getName() const;
            /// \brief Gets the description of the stat.
            std::string getDescription() const;

            /// \brief Prints the stat in \p out (it prints what \p toString returns).
            void print(std::ostream& out);
            /// \brief Returns a string with the contents of the stat.
            /// e.g.: 35::CNOTNum::Number of CNOT nodes.
            virtual std::string toString() = 0;
    };

    /// \brief Stats of a given type.
    /// 
    /// This should be used for collecting statistical results like elapsed time
    /// of some function, or uses of something else.
    template <typename T>
        class Stat : public StatBase {
            private:
                T mVal;

            public:
                Stat(std::string name, std::string description);

                /// \brief Gets a copy of the value stored.
                T getVal() const;

                Stat<T>& operator=(const T val);
                Stat<T>& operator+=(const T val);
                Stat<T>& operator-=(const T val);
                Stat<T>& operator*=(const T val);
                Stat<T>& operator/=(const T val);

                std::string toString() override;
        };

    /// \brief Usually called in the end of the program, i.e. when all statistical
    /// data have already been collected.
    void PrintStats(std::ostream& out = std::cout);
}

template <typename T>
efd::Stat<T>::Stat(std::string name, std::string description) : StatBase(name, description), mVal(0) {
}

template <typename T>
T efd::Stat<T>::getVal() const {
    return mVal;
}

template <typename T>
efd::Stat<T>& efd::Stat<T>::operator=(const T val) {
    mVal = val;
    return *this;
}

template <typename T>
efd::Stat<T>& efd::Stat<T>::operator+=(const T val) {
    mVal += val;
    return *this;
}

template <typename T>
efd::Stat<T>& efd::Stat<T>::operator-=(const T val) {
    mVal -= val;
    return *this;
}

template <typename T>
efd::Stat<T>& efd::Stat<T>::operator*=(const T val) {
    mVal *= val;
    return *this;
}

template <typename T>
efd::Stat<T>& efd::Stat<T>::operator/=(const T val) {
    mVal /= val;
    return *this;
}

template <typename T>
std::string efd::Stat<T>::toString() {
    std::string s;

    s += std::to_string(mVal) + "::";
    s += mName + "::";
    s += mDescription;
    return s;
}

#endif
