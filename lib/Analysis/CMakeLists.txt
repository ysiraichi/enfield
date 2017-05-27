find_package (BISON REQUIRED)
find_package (FLEX REQUIRED)

FLEX_TARGET (EfdScanner Scanner.l "${CMAKE_CURRENT_BINARY_DIR}/EfdScanner.cpp")
BISON_TARGET (EfdParser Parser.yy "${CMAKE_CURRENT_BINARY_DIR}/EfdParser.cpp")
ADD_FLEX_BISON_DEPENDENCY (EfdScanner EfdParser)

# ------- For debug reasons -------- #
configure_file (Parser.yy "${CMAKE_CURRENT_BINARY_DIR}/Parser.yy" COPYONLY)

add_library (EfdAnalysis
    Nodes.cpp
    ${BISON_EfdParser_OUTPUTS}
    ${FLEX_EfdScanner_OUTPUTS}
    ParserHelper.cpp
    NodeVisitor.cpp)
