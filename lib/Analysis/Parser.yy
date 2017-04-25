%skeleton "lalr1.cc"

%defines
%define parser_class_name {EfdParser}
%define api.token.constructor
%define api.value.type variant
%define parse.assert

%locations

%define parse.trace
%define parse.error verbose

%define api.token.prefix {TOK_}

%token END;

%printer { yyoutput << $$; } <*>;

%%

%start program;

program: %empty;

%%

