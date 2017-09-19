#ifndef __EFD_REGISTRY_H__
#define __EFD_REGISTRY_H__

#include <iostream>
#include <map>
#include <functional>
#include <string>
#include <cassert>

namespace efd {
    /// \brief Base registry class for sets.
    ///
    /// This is used by storing Allocators and Architectures.
    template <typename RTy, typename ATy,
             typename KTy = std::string,
             typename CTy = std::less<KTy>>
    class Registry {
        public:
            typedef RTy RetTy;
            typedef ATy ArgTy;
            typedef KTy KeyTy;
            typedef CTy CmpTy;
            typedef std::function<RetTy(ArgTy)> CtorTy;

        private:
            std::map<KeyTy, CtorTy> mCtorMap;

        public:
            /// \brief Registers an object constructor \p ctor with \p key.
            void registerObj(KeyTy key, CtorTy ctor);
            /// \brief Returns true if \p key exists inside \em mCtorMap.
            bool hasObj(KeyTy key) const;
            /// \brief Creates an object instance by calling the constructor mapped
            /// by \p key, with \p arg.
            RetTy createObj(KeyTy key, ArgTy arg) const;
    };
}

template <typename RetTy, typename ArgTy, typename KeyTy, typename CmpTy>
void efd::Registry<RetTy, ArgTy, KeyTy, CmpTy>::registerObj(KeyTy key, CtorTy ctor) {
    if (!hasObj(key)) {
        mCtorMap.insert(std::make_pair(key, ctor));
    } else {
        std::cerr << "Architecture `" << key << "` overrided." << std::endl;
    }
}

template <typename RetTy, typename ArgTy, typename KeyTy, typename CmpTy>
bool efd::Registry<RetTy, ArgTy, KeyTy, CmpTy>::hasObj(KeyTy key) const {
    return mCtorMap.find(key) != mCtorMap.end();
}

template <typename RetTy, typename ArgTy, typename KeyTy, typename CmpTy>
RetTy efd::Registry<RetTy, ArgTy, KeyTy, CmpTy>::createObj(KeyTy key, ArgTy arg) const {
    assert(hasObj(key) && "Trying to create an object not registered.");
    return mCtorMap.at(key)(arg);
}

#endif
