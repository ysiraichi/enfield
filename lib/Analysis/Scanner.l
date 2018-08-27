%{
#include "EfdParser.hpp"
#include <string>

static efd::yy::location loc;

#undef yyFlexLexer

#undef YY_NULL
#define YY_NULL efd::yy::EfdParser::make_EOF(loc)

%}

%option c++
%option noyywrap nounput batch debug noinput
%option yyclass="efd::EfdScanner"
%option nodefault

id      [a-z][A-Za-z0-9_]*
integer [1-9]+[0-9]*|0
exp     [eE][-+]?[0-9]+
frac    \.[0-9]*
real    ({integer}{frac}?|{frac}){exp}?
blank   [ \t]
string  \".*\"

%{
#define YY_USER_ACTION loc.columns(yyleng);
%}

%%

%{
    loc.step();
%}

{blank}+    { loc.step(); }

[\r\n]+     {
                loc.lines(yyleng); 
                loc.step(); 
            }

"//".*      { loc.lines(1); }

"OPENQASM"  { return efd::yy::EfdParser::make_IBMQASM(loc); }
"include"   { return efd::yy::EfdParser::make_INCLUDE(loc); }
"opaque"    { return efd::yy::EfdParser::make_OPAQUE(loc); }
"if"        { return efd::yy::EfdParser::make_IF(loc); }
"barrier"   { return efd::yy::EfdParser::make_BARRIER(loc); }
"qreg"      { return efd::yy::EfdParser::make_QREG(loc); }
"creg"      { return efd::yy::EfdParser::make_CREG(loc); }
"gate"      { return efd::yy::EfdParser::make_GATE(loc); }
"measure"   { return efd::yy::EfdParser::make_MEASURE(loc); }
"reset"     { return efd::yy::EfdParser::make_RESET(loc); }

"CX"        { return efd::yy::EfdParser::make_CX(loc); }
"U"         { return efd::yy::EfdParser::make_U(loc); }

"sin"       { return efd::yy::EfdParser::make_SIN(loc); }
"cos"       { return efd::yy::EfdParser::make_COS(loc); }
"tan"       { return efd::yy::EfdParser::make_TAN(loc); }
"exp"       { return efd::yy::EfdParser::make_EXP(loc); }
"ln"        { return efd::yy::EfdParser::make_LN(loc); }
"sqrt"      { return efd::yy::EfdParser::make_SQRT(loc); }

"=="        { return efd::yy::EfdParser::make_EQUAL(loc); }
"+"         { return efd::yy::EfdParser::make_ADD(loc); }
"-"         { return efd::yy::EfdParser::make_SUB(loc); }
"*"         { return efd::yy::EfdParser::make_MUL(loc); }
"/"         { return efd::yy::EfdParser::make_DIV(loc); }
"^"         { return efd::yy::EfdParser::make_POW(loc); }
"("         { return efd::yy::EfdParser::make_LPAR(loc); }
")"         { return efd::yy::EfdParser::make_RPAR(loc); }
"["         { return efd::yy::EfdParser::make_LSBRAC(loc); }
"]"         { return efd::yy::EfdParser::make_RSBRAC(loc); }
"{"         { return efd::yy::EfdParser::make_LCBRAC(loc); }
"}"         { return efd::yy::EfdParser::make_RCBRAC(loc); }
"->"        { return efd::yy::EfdParser::make_MARROW(loc); }
","         { return efd::yy::EfdParser::make_COMMA(loc); }
";"         { return efd::yy::EfdParser::make_SEMICOL(loc); }

{integer}   { return efd::yy::EfdParser::make_INT(std::string(yytext), loc); }

{real}      { return efd::yy::EfdParser::make_REAL(std::string(yytext), loc); }

{string}    { return efd::yy::EfdParser::make_STRING(std::string(yytext), loc); }

{id}        { return efd::yy::EfdParser::make_ID(yytext, loc); }

.           {}

%%

