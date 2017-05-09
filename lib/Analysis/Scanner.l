%{
    #include <string>
    static efd::yy::location loc;
%}

%option c++
%option noyywrap nounput batch debug noinput

id    [a-z][A-Za-z0-9_]*
real  ([0-9]+\.[0-9]*|[0-9]*\.[0-9]+)([eE][-+]?[0-9]+)?
integer [1-9]+[0-9]*|0
blank [ \t]

%{
    #define YY_USER_ACTION loc.columns(yyleng);
%}

%%

%{
    loc.step();
%}

{blank}+    { loc.step(); }

[\n]+       {
                loc.lines(yyleng); 
                loc.step(); 
            }

"OpenQASM"  { return efd::yy::EfdParser::make_IBMQASM(loc); }
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

{real}      { return efd::yy::EfdParser::make_REAL(yytext, loc); }

{integer}   { return efd::yy::EfdParser::make_INT(yytext, loc); }

{id}        { return efd::yy::EfdParser::make_ID(yytext, loc); }

.           {}

%%

