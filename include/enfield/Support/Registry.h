#ifndef __EFD_REGISTRY_H__
#define __EFD_REGISTRY_H__

#include "enfield/Support/EnumString.h"
#include "enfield/Support/Defs.h"

#include <iostream>
#include <map>
#include <functional>
#include <string>

namespace efd {
    /// \brief Base registry class for sets.
    ///
    /// This is used by storing Allocators and Architectures.
    /// Type-Parameters:
    ///   - RTy: the return type (basically, what we are storing).
    ///   - ATy: the type of the argument of the constructor (for now, we accept only
    ///          one -- we could use std::forward, though).
    ///   - KTy: the key type (it should be an EnumString).
    ///   - CTy: the comparison type.
    template <typename RTy, typename ATy, typename KTy,
             typename CTy = std::less<KTy>>
    class Registry {
        public:
            typedef RTy RetTy;
            typedef ATy ArgTy;
            typedef KTy KeyTy;
            typedef CTy CmpTy;
            typedef std::function<RetTy(ArgTy)> CtorTy;

        private:
            std::map<KeyTy, CtorTy, CmpTy> mCtorMap;

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
        WAR << "Object `" << key.getStringValue() << "` overrided." << std::endl;
    }
}

template <typename RetTy, typename ArgTy, typename KeyTy, typename CmpTy>
bool efd::Registry<RetTy, ArgTy, KeyTy, CmpTy>::hasObj(KeyTy key) const {
    return mCtorMap.find(key) != mCtorMap.end();
}

template <typename RetTy, typename ArgTy, typename KeyTy, typename CmpTy>
RetTy efd::Registry<RetTy, ArgTy, KeyTy, CmpTy>::createObj(KeyTy key, ArgTy arg) const {
    EfdAbortIf(!hasObj(key), "Trying to create an object not registered." );
    return mCtorMap.at(key)(arg);
}

#endif
