#ifndef __EFD_COMMAND_LINE_H__
#define __EFD_COMMAND_LINE_H__

#include <string>
#include <vector>
#include <cassert>
#include <memory>

namespace efd {
    class ArgsParser;

    /// \brief Base class for implementing command line options.
    class OptBase {
        private:
            bool mIsRequired;
            bool mIsParsed;

            std::shared_ptr<ArgsParser> mParser;

        protected:
            /// \brief Type sensitive parsing of the arguments itself.
            virtual void parseImpl(std::vector<std::string> args) = 0;

        public:
            /// \brief Command line string that represents the option.
            std::string mName;
            /// \brief Description of the command line option.
            std::string mDescription;

            /// \brief Constructs and inserts itself in the command line options container.
            OptBase(std::string name, std::string description, bool isRequired);
            /// \brief Destructs and removes itself from the command line options container.
            virtual ~OptBase();

            /// \brief True if this option has already been parsed.
            bool isParsed();
            /// \brief True if the option was constructed as 'required'.
            bool isRequired();

            /// \brief Type sensitive number of arguments consumed.
            virtual uint32_t argsConsumed() = 0;
            /// \brief Return the value in a string representation.
            virtual std::string getStringVal() = 0;
            /// \brief Calls the \em parseImpl function, which is type-dependent.
            void parse(std::vector<std::string> args);
    };

    /// \brief Class used to declare the command line options available.
    template <typename T>
    class Opt : OptBase {
        private:
            T mVal;

        protected:
            void parseImpl(std::vector<std::string> args) override;

        public:
            Opt(std::string name, std::string description, bool isRequired = false);
            Opt(std::string name, std::string description, T def, bool isRequired = false);

            /// \brief Gets a const reference to the value of this command line option.
            const T& getVal() const;
            uint32_t argsConsumed() override;
            std::string getStringVal() override;
    };

    /// \brief Parses the command line arguments (this function should be used in the main
    /// function).
    void ParseArguments(const int argc, const char **argv);
    void ParseArguments(int argc, char **argv);

    template class Opt<bool>;
    template <> uint32_t Opt<bool>::argsConsumed();
    template <> std::string Opt<bool>::getStringVal();
    template <> void Opt<bool>::parseImpl(std::vector<std::string> args);

    template class Opt<int>;
    template <> void Opt<int>::parseImpl(std::vector<std::string> args);

    template class Opt<unsigned>;
    template <> void Opt<unsigned>::parseImpl(std::vector<std::string> args);

    template class Opt<long long>;
    template <> void Opt<long long>::parseImpl(std::vector<std::string> args);

    template class Opt<unsigned long long>;
    template <> void Opt<unsigned long long>::parseImpl(std::vector<std::string> args);

    template class Opt<float>;
    template <> void Opt<float>::parseImpl(std::vector<std::string> args);

    template class Opt<double>;
    template <> void Opt<double>::parseImpl(std::vector<std::string> args);

    template class Opt<std::string>;
    template <> std::string Opt<std::string>::getStringVal();
    template <> void Opt<std::string>::parseImpl(std::vector<std::string> args);

    template class Opt<std::vector<std::string>>;
    template <> std::string Opt<std::vector<std::string>>::getStringVal();
    template <> void Opt<std::vector<std::string>>::parseImpl(std::vector<std::string> args);
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
void efd::Opt<T>::parseImpl(std::vector<std::string> args) {
    assert(false && "Option with 'parse' function not implemented.");
}

template <typename T>
uint32_t efd::Opt<T>::argsConsumed() { return 1; }

template <typename T>
std::string efd::Opt<T>::getStringVal() { return std::to_string(mVal); }

#endif
