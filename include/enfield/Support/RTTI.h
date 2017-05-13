#ifndef __EFD_RTTI_H__
#define __EFD_RTTI_H__

namespace efd {
    
    /// \brief Returns true if the pointer \em src is of class \em TargetTy.
    template <typename TargetTy, typename SrcTy>
        bool instanceOf(const SrcTy* src) {
            return (src) && TargetTy::ClassOf(src);
        }

    /// \brief Returns \em src cast to \em TargetTy, iff it is of class \em TargetTy, otherwise 
    /// it will return nullptr.
    template <typename TargetTy, typename SrcTy>
        TargetTy* dynCast(SrcTy* src) {
            if (src && TargetTy::ClassOf(src))
                return static_cast<TargetTy*>(src);
            return nullptr;
        }
};

#endif
