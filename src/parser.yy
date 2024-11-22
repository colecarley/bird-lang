%skeleton "lalr1.cc"
%require  "3.2"
%debug 
%defines 
%define api.namespace {Bird}
%define api.parser.class {Parser}
%debug


%code requires{
   #include <iostream>
   #include <string>
   #include <vector>
   #include <memory>
   
   #include "../include/ast_node/expr/binary.h"
   #include "../include/ast_node/expr/unary.h"
   #include "../include/ast_node/expr/primary.h"
   #include "../include/ast_node/expr/ternary.h"
   #include "../include/ast_node/expr/call.h"

   #include "../include/ast_node/stmt/decl_stmt.h"
   #include "../include/ast_node/expr/assign_expr.h"
   #include "../include/ast_node/stmt/print_stmt.h"
   #include "../include/ast_node/stmt/if_stmt.h"
   #include "../include/ast_node/stmt/expr_stmt.h"
   #include "../include/ast_node/stmt/const_stmt.h"
   #include "../include/ast_node/stmt/while_stmt.h"
   #include "../include/ast_node/stmt/for_stmt.h"
   #include "../include/ast_node/stmt/return_stmt.h"
   #include "../include/ast_node/stmt/block.h"
   #include "../include/ast_node/stmt/func.h"
   #include "../include/ast_node/stmt/break_stmt.h"
   #include "../include/ast_node/stmt/continue_stmt.h"

   #include "../include/exceptions/bird_exception.h"
   #include "../include/exceptions/user_exception.h"
   #include "../include/exceptions/user_error_tracker.h"


   #define YYDEBUG 1

   namespace Bird {
      class Scanner;
   }
}

%lex-param {Bird::Scanner &scanner}
%parse-param {Bird::Scanner &scanner}
%parse-param {std::vector<std::unique_ptr<Stmt>> &stmts}

%code {
   #include "../src/scanner.hpp"
   static Bird::Parser::token_kind_type yylex(Bird::Parser::semantic_type* value, Bird::Parser::location_type* location, Bird::Scanner& scanner)
   { return scanner.next_token(value, location); }
}

/* %define api.value.type variant */
%define parse.assert

%union {
   int int_val;
   double double_val;
   int bool_val;
   char* string_val;
   char* identifier;

   Stmt* stmt_ptr;
   Expr* expr_ptr;
   // DeclStmt* decl_stmt_ptr;
   // IfStmt* if_stmt_ptr;
   // ConstStmt* const_stmt_ptr;
   // PrintStmt* print_stmt_ptr;
   // Block* block_ptr;
   // Func* func_ptr;
   // WhileStmt* while_stmt_ptr;
   // ForStmt* for_stmt_ptr;
   // ReturnStmt* return_stmt_ptr;
   // BreakStmt* break_stmt_ptr;
   // ContinueStmt* continue_stmt_ptr;
}

%token END 0 _("end of file")
%token VAR "var"
%token CONST "const"
%token MINUS "-"
%token PERCENT "%"
%token PLUS "+"
%token SLASH "/"
%token STAR "*"
%token SEMICOLON ";"
%token QUESTION "?"
%token EQUAL "="
%token PRINT "print"
%token COMMA ","
%token RBRACE "]"
%token LBRACE "["
%token RPAREN ")"
%token LPAREN "("
%token <identifier> IDENTIFIER _("identifier")
%token <identifier> TYPE_IDENTIFIER _("type identifier")
%token <int_val> INT_LITERAL _("int literal")
%token <double_val> FLOAT_LITERAL _("float literal")
%token <bool_val> BOOL_LITERAL _("bool literal")
%token <string_val> STR_LITERAL _("string literal")
%token COLON ":"
%token IF "if"
%token ELSE "else"
%token WHILE "while"
%token FOR "for"
%token DO "do"
%token GREATER ">"
%token LESS "<"
%token BANG "!"
%token ARROW "->"
%token GREATER_EQUAL ">="
%token LESS_EQUAL "<="
%token EQUAL_EQUAL "=="
%token BANG_EQUAL "!="
%token PLUS_EQUAL "+="
%token MINUS_EQUAL "-="
%token STAR_EQUAL "*="
%token SLASH_EQUAL "/="
%token PERCENT_EQUAL "%="
%token RETURN "return"
%token BREAK "break"
%token CONTINUE "continue"
%token FN "fn"


/* %type <stmt_ptr> stmt
%type <stmt_ptr> decl_stmt
%type <stmt_ptr> if_stmt
%type <stmt_ptr> const_stmt
%type <stmt_ptr> print_stmt
%type <stmt_ptr> block
%type <stmt_ptr> func
%type <stmt_ptr> while_stmt
%type <stmt_ptr> for_stmt
%type <stmt_ptr> return_stmt
%type <stmt_ptr> break_stmt
%type <stmt_ptr> continue_stmt
%type <stmt_ptr> expr_stmt

%type <expr_ptr> expr */

%right ASSIGN
%right TERNARY
%left EQUALITY
%left COMPARISON
%left ADD SUBTRACT
%left MULTIPLY DIVIDE
%right UNARY
%left CALL
%nonassoc PRIMARY GROUPING

%nonassoc THEN
%nonassoc ELSE

%locations

%start program

%%

program: stmts

stmts: 
   | stmt stmts

stmt: decl_stmt
   | if_stmt
   | const_stmt
   | print_stmt
   | block
   | func
   | while_stmt
   | for_stmt
   | return_stmt
   | break_stmt
   | continue_stmt
   | expr_stmt

decl_stmt: VAR IDENTIFIER EQUAL expr SEMICOLON
   | VAR IDENTIFIER COLON TYPE_IDENTIFIER EQUAL expr SEMICOLON

if_stmt: IF expr stmt      %prec THEN
   |     IF expr stmt ELSE stmt

const_stmt: CONST IDENTIFIER EQUAL expr SEMICOLON
   | CONST IDENTIFIER COLON TYPE_IDENTIFIER PLUS expr SEMICOLON

print_stmt: PRINT arg_list SEMICOLON

block: LBRACE stmts RBRACE

func: FN IDENTIFIER LPAREN param_list RPAREN return_type block

while_stmt: WHILE expr stmt

for_stmt: FOR maybe_stmt maybe_stmt expr DO stmt

return_stmt: RETURN SEMICOLON
   | RETURN expr SEMICOLON

break_stmt: BREAK SEMICOLON

continue_stmt: CONTINUE SEMICOLON

expr_stmt: expr SEMICOLON

arg_list: 
   | expr
   | expr COMMA arg_list

param_list: 
   | param
   | param COMMA param_list

param: IDENTIFIER COLON TYPE_IDENTIFIER

return_type: 
   | ARROW TYPE_IDENTIFIER

maybe_stmt: SEMICOLON
   | stmt


expr: assign_expr %prec ASSIGN
   | ternary_expr %prec TERNARY
   | equality_expr %prec EQUALITY
   | comparison_expr %prec COMPARISON
   | add_expr %prec ADD
   | subtract_expr %prec SUBTRACT
   | multiply_expr %prec MULTIPLY
   | divide_expr %prec DIVIDE
   | unary_expr %prec UNARY
   | call_expr %prec CALL
   | primary %prec PRIMARY
   | grouping %prec GROUPING


assign_expr: expr ASSIGN_OP expr

ternary_expr: expr QUESTION expr COLON expr

equality_expr: expr EQUALITY_OP expr

comparison_expr: expr COMP_OP expr

add_expr: expr PLUS expr
subtract_expr: expr MINUS expr

multiply_expr: expr STAR expr
divide_expr: expr SLASH expr

unary_expr: UNARY_OP expr

call_expr: expr LPAREN arg_list RPAREN

primary: IDENTIFIER
   | INT_LITERAL
   | FLOAT_LITERAL
   | BOOL_LITERAL
   | STR_LITERAL

grouping: LPAREN expr RPAREN

EQUALITY_OP: EQUAL_EQUAL
   | BANG_EQUAL

ASSIGN_OP: EQUAL
   | PLUS_EQUAL
   | MINUS_EQUAL
   | STAR_EQUAL
   | SLASH_EQUAL
   | PERCENT_EQUAL

COMP_OP: GREATER
   | GREATER_EQUAL
   | LESS
   | LESS_EQUAL

UNARY_OP: MINUS

%%

void Bird::Parser::error( const location_type &loc, const std::string &err_message )
{
   std::cerr << "Error: " << err_message << " at line " << loc << "\n";
   /* std::cerr << "[Tried to parse token " << yylloc.token_kind_type << "]\n"; */
   /* std::cout << "Error with parsing " << std::endl; */
}