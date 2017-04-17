#ifndef __EFD_COMMAND_LINE_H__
#define __EFD_COMMAND_LINE_H__

#include <string>
#include <cassert>

namespace efd {

    class OptBase {
        private:
            bool mIsRequired;
            bool mIsParsed;

        public:
            std::string mName;
            std::string mDescription;

            OptBase(std::string name, std::string description, bool isRequired);

            bool isParsed();
            bool isRequired();

            virtual void parse(int argc, char **argv) = 0;
    };

    template <typename T>
    class Opt : OptBase {
        private:
            T mVal;

        public:
            Opt(std::string name, std::string description, bool isRequired = false);
            Opt(std::string name, std::string description, T def, bool isRequired = false);

            const T& getVal() const;
            void parse(int argc, char **argv) override;
    };

    bool ParseArguments(int argc, char **argv);

    template class Opt<bool>;
    template <> void Opt<bool>::parse(int argc, char **argv);

    template class Opt<int>;
    template <> void Opt<int>::parse(int argc, char **argv);

    template class Opt<unsigned>;
    template <> void Opt<unsigned>::parse(int argc, char **argv);

    template class Opt<long long>;
    template <> void Opt<long long>::parse(int argc, char **argv);

    template class Opt<unsigned long long>;
    template <> void Opt<unsigned long long>::parse(int argc, char **argv);

    template class Opt<float>;
    template <> void Opt<float>::parse(int argc, char **argv);

    template class Opt<double>;
    template <> void Opt<double>::parse(int argc, char **argv);

    template class Opt<std::string>;
    template <> void Opt<std::string>::parse(int argc, char **argv);
};

template <typename T>
efd::Opt<T>::Opt(std::string name, std::string description, bool isRequired) :
    OptBase(name, description, isRequired) {}

template <typename T>
efd::Opt<T>::Opt(std::string name, std::string description, T def, bool isRequired) :
    OptBase(name, description, isRequired), mVal(def) {}

template <typename T>
const T& efd::Opt<T>::getVal() const { return mVal; }

template <typename T>
void efd::Opt<T>::parse(int argc, char **argv) {
    assert(false && "Option with 'parse' function not implemented.");
}

#endif
