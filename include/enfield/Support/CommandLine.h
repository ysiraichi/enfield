#ifndef __EFD_COMMAND_LINE_H__
#define __EFD_COMMAND_LINE_H__

#include <string>

namespace efd {

    class OptBase {
        public:
            std::string mName;
            std::string mDescription;

            OptBase(std::string name, std::string description);

            virtual void setVal(std::string valStr);
    };

    template <typename T>
    class Opt : OptBase {
        private:
            T mVal;

        public:
            Opt(std::string name, std::string description);
            Opt(std::string name, std::string description, T def);

            T& getVal() const;
            void setVal(std::string valStr) override;
    };

    void ParseArguments(int argc, char **argv);

};

template <typename T>
efd::Opt<T>::Opt(std::string name, std::string description) : OptBase(name, description) {}

template <typename T>
efd::Opt<T>::Opt(std::string name, std::string description, T def) : OptBase(name, description), mVal(def) {}

template <typename T>
T& efd::Opt<T>::getVal() const {
    return mVal;
}

#endif
