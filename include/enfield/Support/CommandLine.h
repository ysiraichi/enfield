#ifndef __EFD_COMMAND_LINE_H__
#define __EFD_COMMAND_LINE_H__

#include <string>
#include <cassert>

namespace efd {

    class OptBase {
        public:
            std::string mName;
            std::string mDescription;

            OptBase(std::string name, std::string description);

            virtual void parse(int argc, char **argv) = 0;
    };

    template <typename T>
    class Opt : OptBase {
        private:
            T mVal;

        public:
            Opt(std::string name, std::string description);
            Opt(std::string name, std::string description, T def);

            const T& getVal() const;
            void parse(int argc, char **argv) override;
    };

    void ParseArguments(int argc, char **argv);

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
efd::Opt<T>::Opt(std::string name, std::string description) : OptBase(name, description) {}

template <typename T>
efd::Opt<T>::Opt(std::string name, std::string description, T def) : OptBase(name, description), mVal(def) {}

template <typename T>
const T& efd::Opt<T>::getVal() const { return mVal; }

template <typename T>
void efd::Opt<T>::parse(int argc, char **argv) {
    assert(false && "Option with 'parse' function not implemented.");
}

#endif
