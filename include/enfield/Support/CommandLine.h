#ifndef __EFD_COMMAND_LINE_H__
#define __EFD_COMMAND_LINE_H__

#include "enfield/Support/PossibleValuesListTrait.h"

#include <string>
#include <vector>
#include <memory>

namespace efd {
    class ArgsParser;
    template <typename T> class Opt;

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

            /// \brief Return list of possible values for this option.
            virtual std::vector<std::string> getPossibleValuesList() = 0;
            /// \brief Type sensitive number of arguments consumed.
            virtual uint32_t argsConsumed() = 0;
            /// \brief Return the value in a string representation.
            virtual std::string getStringVal() = 0;
            /// \brief Calls the \em parseImpl function, which is type-dependent.
            void parse(std::vector<std::string> args);
    };

    /// \brief Parses the arguments in \p args to \p opt.
    ///
    /// One should implement this trait in order to have its type parsed by the
    /// command line.
    /// P.S: Traits are the only way to do this. We can not have partial
    /// specialization.
    template <typename T> struct ParseOptTrait {
        static void Run(Opt<T>* opt, std::vector<std::string> args);
    };

    template <> struct ParseOptTrait<bool> {
        static void Run(Opt<bool>* opt, std::vector<std::string> args);
    };

    template <> struct ParseOptTrait<int> {
        static void Run(Opt<int>* opt, std::vector<std::string> args);
    };

    template <> struct ParseOptTrait<unsigned> {
        static void Run(Opt<unsigned>* opt, std::vector<std::string> args);
    };

    template <> struct ParseOptTrait<long long> {
        static void Run(Opt<long long>* opt, std::vector<std::string> args);
    };

    template <> struct ParseOptTrait<unsigned long long> {
        static void Run(Opt<unsigned long long>* opt, std::vector<std::string> args);
    };

    template <> struct ParseOptTrait<float> {
        static void Run(Opt<float>* opt, std::vector<std::string> args);
    };

    template <> struct ParseOptTrait<double> {
        static void Run(Opt<double>* opt, std::vector<std::string> args);
    };

    template <> struct ParseOptTrait<std::string> {
        static void Run(Opt<std::string>* opt, std::vector<std::string> args);
    };

    template <> struct ParseOptTrait<std::vector<std::string>> {
        static void Run(Opt<std::vector<std::string>>* opt, std::vector<std::string> args);
    };

    template <typename T, T first, T last, uint32_t padding>
        struct ParseOptTrait<EnumString<T, first, last, padding>> {
            static void Run(Opt<EnumString<T, first, last, padding>>* opt,
                            std::vector<std::string> args);
        };

    /// \brief Class used to declare the command line options available.
    template <typename T>
    class Opt : public OptBase {
        private:
            T mVal;

        protected:
            void parseImpl(std::vector<std::string> args) override;

        public:
            Opt(std::string name, std::string description, bool isRequired = false);
            Opt(std::string name, std::string description, T def, bool isRequired = false);

            /// \brief Gets a const reference to the value of this command line option.
            const T& getVal() const;

            std::vector<std::string> getPossibleValuesList() override;
            uint32_t argsConsumed() override;
            std::string getStringVal() override;

            friend struct ParseOptTrait<T>;
    };

    /// \brief Parses the command line arguments (this function should be used in the main
    /// function).
    void ParseArguments(const int argc, const char **argv);
    void ParseArguments(int argc, char **argv);

    template <> uint32_t Opt<bool>::argsConsumed();
    template <> std::string Opt<bool>::getStringVal();
    template <> std::string Opt<std::string>::getStringVal();
    template <> std::string Opt<std::vector<std::string>>::getStringVal();
};

template <typename T>
void efd::ParseOptTrait<T>::Run(Opt<T>* opt, std::vector<std::string> args) {
    EfdAbortIf(true, "Parse method not implemented for '" << typeid(T).name() << "'.");
}

template <typename T, T first, T last, uint32_t padding>
void efd::ParseOptTrait<efd::EnumString<T, first, last, padding>>::
Run(Opt<efd::EnumString<T, first, last, padding>>* opt, std::vector<std::string> args) {
    opt->mVal = args[0];
}

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
    ParseOptTrait<T>::Run(this, args);
}

template <typename T>
std::vector<std::string> efd::Opt<T>::getPossibleValuesList() {
    return PossibleValuesListTrait<T>::Get();
}

template <typename T>
uint32_t efd::Opt<T>::argsConsumed() { return 1; }

template <typename T>
std::string efd::Opt<T>::getStringVal() { return std::to_string(mVal); }

#endif
