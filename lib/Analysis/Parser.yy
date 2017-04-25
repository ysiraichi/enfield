%skeleton "lalr1.cc"

%code {
    #include <string>

    // TODO:
    // Move this struct outside.
    namespace efd {
        struct DoubleVal {
            double mV;
            std::string mStr;

            DoubleVal(std::string s) : mStr(s) {
                mV = std::stod(s);
            }
        };
    };
}

%defines
%define parser_class_name {EfdParser}
%define api.namespace {efd::yy}
%define api.token.constructor
%define api.value.type variant
%define parse.assert

%locations

%define parse.trace
%define parse.error verbose

%define api.token.prefix {TOK_}

%token IBMQASM;
%token QREG CREG;
%token OPAQUE GATE;
%token MEASURE BARRIER RESET IF;

%token U CX;
%token SIN COS TAN EXP LN SQRT;
%token EQUAL ADD SUB MUL DIV POW;
%token LPAR RPAR LSBRAC RSBRAC;
%token MARROW COMMA;

%token REAL ID;

%type <std::string> ID;
%type <efd::DoubleVal> REAL;

%printer { yyoutput << $$; } <*>;

%%

%start program;

program: %empty;

%%

