%skeleton "lalr1.cc"

%code requires {
    #include "enfield/Analysis/Nodes.h"
    #include "enfield/Support/WrapperVal.h"
    #include "enfield/Support/RTTI.h"

    typedef efd::Node::NodeRef NodeRef;
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

%token IBMQASM INCLUDE;
%token QREG CREG;
%token OPAQUE GATE;
%token MEASURE BARRIER RESET IF;

%token U CX;
%token SIN COS TAN EXP LN SQRT;

%token EQUAL    "=="
%token ADD      "+"
%token SUB      "-"
%token MUL      "*"
%token DIV      "/"
%token POW      "^"

%token LPAR     "("
%token RPAR     ")"
%token LSBRAC   "["
%token RSBRAC   "]"
%token LCBRAC   "{"
%token RCBRAC   "}"

%token MARROW   "->"
%token COMMA    ","
%token SEMICOL  ";"

%token <efd::IntVal> INT;
%token <efd::RealVal> REAL;
%token <std::string> ID;

%type <NodeRef> program program_ statement
%type <NodeRef> decl gatedecl opaquedecl qop uop
%type <NodeRef> barrier reset measure
%type <NodeRef> goplist
%type <NodeRef> explist explist_ exp
%type <NodeRef> anylist anylist_
%type <NodeRef> idlist idlist_
%type <NodeRef> params args arg
%type <NodeRef> id idref integer real

%type <efd::NDUnaryOp::UOpType> unary

%left "+" "-"
%left "*" "/"
%left "^"
%right NEG

%%

%start program;

program: program_ statement    {
                                    efd::dynCast<efd::NDList>($1.get())->addChild(std::move($2));
                                    $$ = std::move($1);
                                }
       ;
program_: %empty                { $$ = efd::NDList::create(); }
        | program_ statement    {
                                    efd::dynCast<efd::NDList>($1.get())->addChild(std::move($2));
                                    $$ = std::move($1);
                                }

statement: decl         { $$ = std::move($1); }
         | gatedecl     { $$ = std::move($1); }
         | opaquedecl   { $$ = std::move($1); }
         | qop          { $$ = std::move($1); }
         | barrier      { $$ = std::move($1); }
         ;

decl: QREG id "[" integer "]" ";"   { 
                                        $$ = efd::NDDecl::create
                                        (efd::NDDecl::QUANTUM, std::move($2), std::move($4));
                                    }
    | CREG id "[" integer "]" ";"   { 
                                        $$ = efd::NDDecl::create
                                        (efd::NDDecl::CONCRETE, std::move($2), std::move($4));
                                    }
    ;

gatedecl: GATE id params idlist "{" goplist "}" { 
                                                    $$ = efd::NDGateDecl::create
                                                    (std::move($2), std::move($3), 
                                                    std::move($4), std::move($6));
                                                }
        ;

opaquedecl: OPAQUE id params idlist ";" { 
                                            $$ = efd::NDOpaque::create
                                            (std::move($2), std::move($2), std::move($3));
                                        }
          ;

barrier: BARRIER idlist ";" { $$ = efd::NDQOpBarrier::create(std::move($2)); }
       ;

reset: RESET arg ";"    { $$ = efd::NDQOpReset::create(std::move($2)); }
     ;

measure: MEASURE arg "->" arg ";"   { $$ = efd::NDQOpMeasure::create(std::move($2), std::move($4)); }
       ;

params: %empty          { $$ = efd::NDList::create(); }
      | "(" idlist ")"  { $$ = std::move($2); }
      ;

goplist: %empty             { $$ = efd::NDGOpList::create(); }
       | goplist uop        {
                                efd::dynCast<efd::NDList>($1.get())->addChild(std::move($2));
                                $$ = std::move($1);
                            }
       | goplist barrier    {
                                efd::dynCast<efd::NDList>($1.get())->addChild(std::move($2));
                                $$ = std::move($1);
                            }
       ;

qop: uop        { $$ = std::move($1); }
   | measure    { $$ = std::move($1); }
   | reset      { $$ = std::move($1); }
   ;

uop: id args anylist ";"    { 
                                $$ = efd::NDQOpGeneric::create
                                (std::move($1), std::move($2), std::move($3)); 
                            }
   ;

args: %empty            { $$ = efd::NDList::create(); }
    | "(" explist ")"   { $$ = std::move($2); }
    ;

idlist: idlist_ id      {
                            efd::dynCast<efd::NDList>($1.get())->addChild(std::move($2));
                            $$ = std::move($1);
                        }
      ;
idlist_: %empty         { $$ = efd::NDList::create(); }
       | idlist_ id "," {
                            efd::dynCast<efd::NDList>($1.get())->addChild(std::move($2));
                            $$ = std::move($1);
                        }
       ;

anylist: anylist_ id    {
                            efd::dynCast<efd::NDList>($1.get())->addChild(std::move($2));
                            $$ = std::move($1);
                        }
       | anylist_ idref {
                            efd::dynCast<efd::NDList>($1.get())->addChild(std::move($2));
                            $$ = std::move($1);
                        }
       ;
anylist_: %empty                { $$ = efd::NDList::create(); }
        | anylist_ id ","       {
                                    efd::dynCast<efd::NDList>($1.get())->addChild(std::move($2));
                                    $$ = std::move($1);
                                }
        | anylist_ idref ","    {
                                    efd::dynCast<efd::NDList>($1.get())->addChild(std::move($2));
                                    $$ = std::move($1);
                                }
        ;

explist: %empty         { $$ = efd::NDList::create(); }
       | explist_ exp   {
                            efd::dynCast<efd::NDList>($1.get())->addChild(std::move($2));
                            $$ = std::move($1);
                        }
       ;
explist_: %empty            { $$ = efd::NDList::create(); }
        | explist_ exp ","  {
                                efd::dynCast<efd::NDList>($1.get())->addChild(std::move($2));
                                $$ = std::move($1);
                            }

exp: real               { $$ = std::move($1); }
   | integer            { $$ = std::move($1); }
   | id                 { $$ = std::move($1); }
   | unary "(" exp ")"  { $$ = efd::NDUnaryOp::create($1, std::move($3)); }
   | "(" exp ")"        { $$ = std::move($2); }
   | exp "+" exp        { $$ = efd::NDBinOp::create(efd::NDBinOp::OP_ADD, std::move($1), std::move($3)); }
   | exp "-" exp        { $$ = efd::NDBinOp::create(efd::NDBinOp::OP_SUB, std::move($1), std::move($3)); }
   | exp "*" exp        { $$ = efd::NDBinOp::create(efd::NDBinOp::OP_MUL, std::move($1), std::move($3)); }
   | exp "/" exp        { $$ = efd::NDBinOp::create(efd::NDBinOp::OP_DIV, std::move($1), std::move($3)); }
   | exp "^" exp        { $$ = efd::NDBinOp::create(efd::NDBinOp::OP_POW, std::move($1), std::move($3)); }
   ;

unary: SIN  { $$ = efd::NDUnaryOp::UOP_SIN; }
     | COS  { $$ = efd::NDUnaryOp::UOP_COS; }
     | TAN  { $$ = efd::NDUnaryOp::UOP_TAN; }
     | EXP  { $$ = efd::NDUnaryOp::UOP_EXP; }
     | LN   { $$ = efd::NDUnaryOp::UOP_LN; }
     | SQRT { $$ = efd::NDUnaryOp::UOP_SQRT; }
     ;

arg: id     { $$ = std::move($1); }
   | idref  { $$ = std::move($1); }
   ;

idref: id "[" integer "]" { $$ = efd::NDIdRef::create(std::move($1), std::move($3)); }
     ;

id: ID { $$ = efd::NDId::create($1); }
  ;

integer: INT { $$ = efd::NDInt::create($1); }
       ;

real: REAL { $$ = efd::NDReal::create($1); }
    ;

%%

