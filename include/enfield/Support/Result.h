#ifndef __EFD_RESULTS_H__
#define __EFD_RESULTS_H__

#include <string>

namespace efd {
    template <typename ErrorTy>
        class Result {
            private:
                bool mError;
                ErrorTy mMessage;

            public:
                bool isError() const { return mError; }
                bool isSuccess() const { return !mError; }
                std::string getErrorMessage() const { return mMessage; }

                static Result<ErrorTy> Success() {
                    Result<ErrorTy> r;
                    r.mError = false;
                    return r;
                }

                static Result<ErrorTy> Error(ErrorTy error) {
                    Result<ErrorTy> r;
                    r.mError = true;
                    r.mMessage = error;
                    return r;
                }
        };

    using ResultMsg = Result<std::string>;
}

#endif
