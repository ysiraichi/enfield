#ifndef __EFD_JSON_PARSER_H__
#define __EFD_JSON_PARSER_H__

#include "enfield/Support/Defs.h"

#include <memory>
#include <sstream>
#include <fstream>
#include <json/json.h>

namespace efd {
    /// \brief Centralizes the field names for each type.
    template <class T> struct JsonFields {};

    /// \brief Returns a \em std::string corresponding to \p ty.
    std::string JsonTypeString(const Json::ValueType& ty);

    /// \brief Returns the \em std::string version of all \p tys for error messages.
    std::string JsonTypeVectorString(const std::vector<Json::ValueType>& tys);

    /// \brief Checks for a type error inside some key of the \em Json::Value.
    template <class T>
        void JsonCheckTypeError(const std::string prefix,
                                const Json::Value v,
                                const T key,
                                const std::vector<Json::ValueType> tys) {
            bool check = false;
            auto keyTy = v[key].type();

            for (auto ty : tys) {
                if (keyTy == ty) {
                    check = true;
                    break;
                }
            }

            EfdAbortIf(!check,
                       prefix << ": incorrect type for " << JsonTypeVectorString(tys)
                       << ". Actual: `" << JsonTypeString(keyTy) << "`.");
        }

    /// \brief Gets a \em T instance (wrapped in a \em std::unique_ptr) from the \em Json::Value.
    template <class T> struct JsonBackendParser {
        static std::unique_ptr<T> Parse(const Json::Value& root) {
            EfdAbortIf(true, "Parse method not implemented for '" << typeid(T).name() << "'.");
        }
    };

    /// \brief Frontend to the json parser.
    template <class T> class JsonParser {
        private:
            static std::unique_ptr<T> ParseInputStream(std::istream& in) {
                Json::Value root;
                in >> root;
                return JsonBackendParser<T>::Parse(root);
            }

        public:
            /// \brief Parses a json \em std::string.
            static std::unique_ptr<T> ParseString(std::string str) {
                std::istringstream iss(str);
                return ParseInputStream(iss);
            }

            /// \brief Parses a json file.
            static std::unique_ptr<T> ParseFile(std::string filename) {
                std::ifstream ifs(filename.c_str());
                return ParseInputStream(ifs);
            }
    };
}

#endif
