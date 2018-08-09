#ifndef __EFD_JSON_PARSER_H__
#define __EFD_JSON_PARSER_H__

#include "enfield/Support/Defs.h"

#include <memory>
#include <json/json.h>

namespace efd {
    /// \brief Centralizes the field names for each type.
    template <class T> struct JsonFields {};

    /// \brief Generates a \em Json::Value object from an input stream for
    /// an object \em T.
    ///
    /// It allows some base checks too.
    template <class T> struct JsonInputParser {
        static Json::Value Parse(std::istream& in) {
            Json::Value root;
            in >> root;
            return root;
        }
    };

    /// \brief Gets a \em T instance (wrapped in a \em std::unique_ptr) from the \em Json::Value.
    template <class T> struct FromJsonGetter {
        static std::unique_ptr<T> Get(const Json::Value& root) {
            ERR << "Get method not implemented for '" << typeid(T).name() << "'." << std::endl;
            ExitWith(ExitCode::EXIT_unreachable);
        }
    };
}

#endif
