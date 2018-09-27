#ifndef __EFD_U_REF_CAST_H__
#define __EFD_U_REF_CAST_H__

#include "enfield/Support/RTTI.h"
#include "enfield/Support/Defs.h"

#include <memory>
#include <typeinfo>

namespace efd {
    /// \brief Uses the RTTI framework to cast backwardly an unique_ptr.
    /// 
    /// Note that it transfers the ownership. So, if the cast returns an
    /// error, by assertion the execution stops.
    ///
    /// It casts from a derived class to a base one.
    template <typename T, typename U>
        std::unique_ptr<T> uniqueCastBackward(std::unique_ptr<U> from) {
            auto fromRef = from.release();

            if (std::is_base_of<T, U>::value) {
                T* toRef = static_cast<T*>(fromRef);
                return std::unique_ptr<T>(toRef);
            }

            EfdAbortIf(true,
                       "Failed when casting a std::unique_ptr. `" << typeid(T).name()
                       << "` not a base class of `" << typeid(U).name() << "`.");
        }

    /// \brief Uses the RTTI framework to cast forwardly an unique_ptr.
    /// 
    /// Note that it transfers the ownership. So, if the cast returns an
    /// error, by assertion the execution stops.
    ///
    /// It casts from a base class to a derived one.
    template <typename T, typename U>
        std::unique_ptr<T> uniqueCastForward(std::unique_ptr<U> from) {
            auto fromRef = from.release();

            if (auto toRef = dynCast<T>(fromRef))
                return std::unique_ptr<T>(toRef);

            EfdAbortIf(true,
                       "Failed when casting a std::unique_ptr: from `" << typeid(U).name()
                       << "` to `" << typeid(T).name() << "`.");
        }

    /// \brief Wrapper function that "transforms" a \em std::unique_ptr
    /// into a \em std::shared_ptr.
    ///
    /// Note that the unique_ptr will lose the reference to the previously
    /// pointed object.
    template <typename T>
        inline std::shared_ptr<T> toShared(std::unique_ptr<T> from) {
            return std::shared_ptr<T>(from.release());
        }
}

#endif
