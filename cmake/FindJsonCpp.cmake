include(FindPackageHandleStandardArgs)

find_library (JSONCPP_MAIN "jsoncpp"
    HINTS
        ENV JSONCPP_ROOT
        ${JSONCPP_ROOT}
    PATH_SUFFIXES
        lib64
        lib)

find_path (JSONCPP_INCLUDE "json/json.h"
    HINTS
        $ENV{JSONCPP_ROOT}/include
        ${JSONCPP_ROOT}/include)

find_package_handle_standard_args (JsonCpp
    DEFAULT_MSG
    JSONCPP_MAIN
    JSONCPP_INCLUDE)
