%skeleton "lalr1.cc"

%code requires {
    #include "enfield/Analysis/Nodes.h"
    #include "enfield/Analysis/Driver.h"
    #include "enfield/Support/WrapperVal.h"
    #include "enfield/Support/RTTI.h"
    #include "enfield/Support/Defs.h"
    #include "enfield/Support/CommandLine.h"

    namespace efd {
        class EfdScanner;
    };
}

%code provides {
    #include <unordered_map>
    #include <sstream>

    extern efd::Opt<std::vector<std::string>> IncludePath;
    extern std::unordered_map<std::string, std::string> StdLib;
}

%code provides {
    #if ! defined(yyFlexLexerOnce)
    #include <FlexLexer.h>
    #endif

    #define YYDEBUG 1

    #include <cstdio>
    #include <cstring>
    #include <cerrno>

    /* Size of default input buffer. */
    #ifndef YY_BUF_SIZE
    #ifdef __ia64__
    /* On IA-64, the buffer size is 16k, not 8k.
     * Moreover, YY_BUF_SIZE is 2*YY_READ_BUF_SIZE in the general case.
     * Ditto for the __ia64__ case accordingly.
     */
    #define YY_BUF_SIZE 32768
    #else
    #define YY_BUF_SIZE 16384
    #endif /* __ia64__ */
    #endif

    #include <iostream>
    #include <fstream>

    namespace efd {
        class EfdScanner : public yyFlexLexer {
            public:
                EfdScanner(std::istream* iStream, std::ostream* oStream);
                yy::EfdParser::symbol_type lex();
        };
    }

    #undef YY_DECL
    #define YY_DECL \
        efd::yy::EfdParser::symbol_type efd::EfdScanner::lex()
}

%code {
    efd::EfdScanner::EfdScanner(std::istream* iStream, std::ostream* oStream)
       : yyFlexLexer(iStream, oStream) {
    }

    void efd::yy::EfdParser::error(efd::yy::location const& loc, std::string const& err) {
        std::string filename = "unknown";
        if (loc.begin.filename) filename = *loc.begin.filename;
        std::cerr << filename << ":" << loc.begin.line << ":" 
            << loc.begin.column << ": " << err << std::endl;
    }
}

%defines
%define parser_class_name {EfdParser}
%define api.token.constructor
%define api.namespace {efd::yy}
%define api.value.type variant
%define parse.assert

%parse-param { efd::ASTWrapper &ast }
%parse-param { efd::EfdScanner &scanner }

%code {
    #define _YYLEX_ scanner.lex
    #define yylex _YYLEX_
}

%locations
%initial-action {
    @$.begin.filename = &ast.mFile;
    @$.end.filename = &ast.mFile;
}

%define parse.trace true
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
%token <std::string> STRING;

%token EOF 0 "end of file";

%type <efd::Node::Ref> program statement
%type <efd::Node::Ref> include decl gatedecl opaquedecl ifstmt
%type <efd::Node::Ref> exp arg

%type <efd::NDQOp::Ref> qop uop barrier reset measure

%type <efd::NDStmtList::Ref> stmtlist stmtlist_
%type <efd::NDGOpList::Ref> goplist
%type <efd::NDList::Ref> explist explist_
%type <efd::NDList::Ref> anylist anylist_
%type <efd::NDList::Ref> idlist idlist_
%type <efd::NDList::Ref> params args

%type <efd::NDIdRef::Ref> idref
%type <efd::NDId::Ref> id string
%type <efd::NDInt::Ref> integer
%type <efd::NDReal::Ref> real

%type <efd::NDUnaryOp::UOpType> unary

%left "+" "-"
%left "*" "/"
%left "^"
%right NEG
%%

%start program;

program: stmtlist                   { $$ = $1; ast.mAST = $$; }
       | IBMQASM real ";" stmtlist  { 
                                        $$ = efd::NDQasmVersion::Create
                                        (efd::NDReal::uRef($2), efd::NDStmtList::uRef($4))
                                        .release();

                                        ast.mAST = $$;
                                    }

stmtlist: stmtlist_ EOF { $$ = $1; }
       ;
stmtlist_: %empty                { $$ = efd::NDStmtList::Create().release(); }
         | stmtlist_ statement   {
                                     efd::dynCast<efd::NDStmtList>($1)->addChild(efd::Node::uRef($2));
                                     $$ = $1;
                                 }

statement: decl         { $$ = $1; }
         | gatedecl     { $$ = $1; }
         | opaquedecl   { $$ = $1; }
         | qop          { $$ = $1; }
         | barrier      { $$ = $1; }
         | ifstmt       { $$ = $1; }
         | include      { $$ = $1; }
         ;

include: INCLUDE string ";"     {
                                    std::istream* istr = nullptr;
                                    std::ifstream ifs;
                                    std::istringstream iss;
                                    efd::ASTWrapper _ast;

                                    std::string file = efd::dynCast<efd::NDString>($2)->getVal();

                                    if (StdLib.find(file) != StdLib.end()) {
                                        _ast = efd::ASTWrapper { file, "./", nullptr, false };
                                        iss.str(StdLib[file]);
                                        istr = &iss;
                                        ast.mStdLibParsed = true;
                                    } else {
                                        std::vector<std::string> includePaths = IncludePath.getVal();
                                        includePaths.push_back(ast.mPath);

                                        for (auto path : includePaths) {
                                            _ast = efd::ASTWrapper { file, path, nullptr, false };
                                            ifs.open((_ast.mPath + _ast.mFile).c_str());
                                            if (!ifs.fail()) {
                                                istr = &ifs;
                                                break;
                                            }
                                        }
                                    }

                                    if (istr == nullptr) {
                                        error(@$, "Could not open file: " + _ast.mPath + _ast.mFile);
                                        error(@$, "Error: " + std::string(strerror(errno)));
                                        return 1;
                                    }

                                    efd::yy::EfdParser parser(_ast, scanner);
                                    scanner.yypush_buffer_state(scanner.yy_create_buffer(istr, YY_BUF_SIZE));
                                    if (parser.parse()) return 1;
                                    scanner.yypop_buffer_state();

                                    $$ = efd::NDInclude::Create(efd::NDString::uRef($2), efd::Node::uRef(_ast.mAST)).release();
                                }
        ;

decl: QREG id "[" integer "]" ";"   { $$ = efd::NDRegDecl::CreateQ(efd::NDId::uRef($2), efd::NDInt::uRef($4)).release(); }
    | CREG id "[" integer "]" ";"   { $$ = efd::NDRegDecl::CreateC(efd::NDId::uRef($2), efd::NDInt::uRef($4)).release(); }
    ;

gatedecl: GATE id params idlist "{" goplist "}" { 
                                                    $$ = efd::NDGateDecl::Create
                                                    (efd::NDId::uRef($2), efd::NDList::uRef($3), 
                                                    efd::NDList::uRef($4), efd::NDGOpList::uRef($6))
                                                    .release();
                                                }
        ;

opaquedecl: OPAQUE id params idlist ";" {
                                            $$ = efd::NDOpaque::Create
                                            (efd::NDId::uRef($2), efd::NDList::uRef($3), efd::NDList::uRef($4))
                                            .release();
                                        }
          ;

barrier: BARRIER anylist ";" { $$ = efd::NDQOpBarrier::Create(efd::NDList::uRef($2)).release(); }
       ;

reset: RESET arg ";"    { $$ = efd::NDQOpReset::Create(efd::Node::uRef($2)).release(); }
     ;

measure: MEASURE arg "->" arg ";"   { $$ = efd::NDQOpMeasure::Create(efd::Node::uRef($2), efd::Node::uRef($4)).release(); }
       ;

ifstmt: IF "(" id "==" integer ")" qop  {
                                            $$ = efd::NDIfStmt::Create
                                            (efd::NDId::uRef($3), efd::NDInt::uRef($5), efd::NDQOp::uRef($7))
                                            .release();
                                        }

params: %empty          { $$ = efd::NDList::Create().release(); }
      | "(" idlist ")"  { $$ = $2; }
      ;

goplist: %empty             { $$ = efd::NDGOpList::Create().release(); }
       | goplist uop        {
                                efd::dynCast<efd::NDGOpList>($1)->addChild(efd::Node::uRef($2));
                                $$ = $1;
                            }
       | goplist barrier    {
                                efd::dynCast<efd::NDGOpList>($1)->addChild(efd::Node::uRef($2));
                                $$ = $1;
                            }
       ;

qop: uop        { $$ = $1; }
   | measure    { $$ = $1; }
   | reset      { $$ = $1; }
   ;

uop: id args anylist ";"    {
                                $$ = efd::NDQOpGen::Create
                                (efd::NDId::uRef($1), efd::NDList::uRef($2), efd::NDList::uRef($3))
                                .release();
                            }
   | U args arg ";"         {
                                $$ = efd::NDQOpU::Create
                                (efd::NDList::uRef($2), efd::Node::uRef($3))
                                .release();
                            }
   | CX arg "," arg ";"     {
                                $$ = efd::NDQOpCX::Create
                                (efd::Node::uRef($2), efd::Node::uRef($4))
                                .release();
                            }
   ;

args: %empty            { $$ = efd::NDList::Create().release(); }
    | "(" explist ")"   { $$ = $2; }
    ;

idlist: idlist_ id      {
                            efd::dynCast<efd::NDList>($1)->addChild(efd::Node::uRef($2));
                            $$ = $1;
                        }
      ;
idlist_: %empty         { $$ = efd::NDList::Create().release(); }
       | idlist_ id "," {
                            efd::dynCast<efd::NDList>($1)->addChild(efd::Node::uRef($2));
                            $$ = $1;
                        }
       ;

anylist: anylist_ id    {
                            efd::dynCast<efd::NDList>($1)->addChild(efd::Node::uRef($2));
                            $$ = $1;
                        }
       | anylist_ idref {
                            efd::dynCast<efd::NDList>($1)->addChild(efd::Node::uRef($2));
                            $$ = $1;
                        }
       ;
anylist_: %empty                { $$ = efd::NDList::Create().release(); }
        | anylist_ id ","       {
                                    efd::dynCast<efd::NDList>($1)->addChild(efd::Node::uRef($2));
                                    $$ = $1;
                                }
        | anylist_ idref ","    {
                                    efd::dynCast<efd::NDList>($1)->addChild(efd::Node::uRef($2));
                                    $$ = $1;
                                }
        ;

explist: %empty         { $$ = efd::NDList::Create().release(); }
       | explist_ exp   {
                            efd::dynCast<efd::NDList>($1)->addChild(efd::Node::uRef($2));
                            $$ = $1;
                        }
       ;
explist_: %empty            { $$ = efd::NDList::Create().release(); }
        | explist_ exp ","  {
                                efd::dynCast<efd::NDList>($1)->addChild(efd::Node::uRef($2));
                                $$ = $1;
                            }

exp: real               { $$ = $1; }
   | integer            { $$ = $1; }
   | id                 { $$ = $1; }
   | unary "(" exp ")"  { $$ = efd::NDUnaryOp::Create($1, efd::Node::uRef($3)).release(); }
   | "(" exp ")"        { $$ = $2; }
   | exp "+" exp        { $$ = efd::NDBinOp::CreateAdd(efd::Node::uRef($1), efd::Node::uRef($3)).release(); }
   | exp "-" exp        { $$ = efd::NDBinOp::CreateSub(efd::Node::uRef($1), efd::Node::uRef($3)).release(); }
   | exp "*" exp        { $$ = efd::NDBinOp::CreateMul(efd::Node::uRef($1), efd::Node::uRef($3)).release(); }
   | exp "/" exp        { $$ = efd::NDBinOp::CreateDiv(efd::Node::uRef($1), efd::Node::uRef($3)).release(); }
   | exp "^" exp        { $$ = efd::NDBinOp::CreatePow(efd::Node::uRef($1), efd::Node::uRef($3)).release(); }
   | "-" exp            { $$ = efd::NDUnaryOp::CreateNeg(efd::Node::uRef($2)).release(); }
   ;

unary: SIN  { $$ = efd::NDUnaryOp::UOP_SIN; }
     | COS  { $$ = efd::NDUnaryOp::UOP_COS; }
     | TAN  { $$ = efd::NDUnaryOp::UOP_TAN; }
     | EXP  { $$ = efd::NDUnaryOp::UOP_EXP; }
     | LN   { $$ = efd::NDUnaryOp::UOP_LN; }
     | SQRT { $$ = efd::NDUnaryOp::UOP_SQRT; }
     ;

arg: id     { $$ = $1; }
   | idref  { $$ = $1; }
   ;

idref: id "[" integer "]"   {
                                $$ = efd::NDIdRef::Create
                                (efd::NDId::uRef($1), efd::NDInt::uRef($3))
                                .release();
                            }
     ;

id: ID { $$ = efd::NDId::Create($1).release(); }
  ;

integer: INT { $$ = efd::NDInt::Create($1).release(); }
       ;

string: STRING { $$ = efd::NDString::Create($1.substr(1, $1.length() - 2)).release(); }

real: REAL { $$ = efd::NDReal::Create($1).release(); }
    ;

%%

static int Parse(std::istream& istr, efd::ASTWrapper& ast, bool forceStdLib) {
    efd::EfdScanner scanner(&istr, nullptr);
    efd::yy::EfdParser parser(ast, scanner);

    int ret = parser.parse();

    if (!ret && forceStdLib && !ast.mStdLibParsed) {
        efd::NDStmtList* stmts = nullptr;
        if (auto versionNode = efd::dynCast<efd::NDQasmVersion>(ast.mAST))
            stmts = versionNode->getStatements();
        else if (auto stmtsNode = efd::dynCast<efd::NDStmtList>(ast.mAST))
            stmts = stmtsNode;
        else {
            // One should not be able to reach this point.
            // This means that the first node of the AST is neither a
            // statements list nor a qasm version node.
            EfdAbortIf(true, "AST Root node is neither a `NDQasmVersion` nor a `NDStmtList`.");
        }

        for (auto pair : StdLib) {
            auto refInclude = efd::NDInclude::Create
                (efd::NDString::Create(pair.first), efd::ParseString(pair.second, false));
            auto inclNode = efd::Node::uRef(refInclude.release());

            auto it = stmts->begin();
            stmts->addChild(it, std::move(inclNode));
        }
    }

    return ret;
}

efd::Node::uRef efd::ParseFile(std::string filename, std::string path, bool forceStdLib) {
    ASTWrapper ast { filename, path, nullptr, false };

    std::ifstream ifs((ast.mPath + ast.mFile).c_str());
    if (ifs.fail()) {
        std::cerr << "Could not open file: " << ast.mPath + ast.mFile << std::endl;
        std::cerr << "Error: " << strerror(errno) << std::endl;
        return nullptr;
    }

    if (Parse(ifs, ast, forceStdLib)) return efd::Node::uRef(nullptr);

    ifs.close();
    return efd::Node::uRef(ast.mAST);
}

efd::Node::uRef efd::ParseString(std::string program, bool forceStdLib) {
    std::string filename = "qasm-" + std::to_string((uint64_t) &program) + ".qasm";
    std::string path =  "./";
    std::stringstream ss(program);

    ASTWrapper ast { filename, path, nullptr, false };
    int ret = Parse(ss, ast, forceStdLib);

    if (ret) return efd::Node::uRef(nullptr);
    return efd::Node::uRef(ast.mAST);
}
