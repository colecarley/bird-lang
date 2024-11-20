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
   #define YYDEBUG 1

   namespace Bird {
      class Scanner;
   }
}

%lex-param {Bird::Scanner &scanner}
%parse-param {Bird::Scanner &scanner}

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
%token FN "fn  "


%locations

%start program

%%

program: tokens

tokens: 
   | token tokens

token:   
IDENTIFIER 
| VAR 
| CONST 
| MINUS 
| PERCENT 
| PLUS 
| SLASH 
| STAR 
| SEMICOLON 
| QUESTION 
| EQUAL 
| PRINT 
| COMMA 
| RBRACE 
| LBRACE 
| RPAREN 
| LPAREN 
| TYPE_IDENTIFIER 
| INT_LITERAL 
| FLOAT_LITERAL 
| BOOL_LITERAL 
| STR_LITERAL 
| COLON 
| IF 
| ELSE 
| WHILE 
| FOR 
| DO 
| GREATER 
| LESS 
| BANG 
| ARROW 
| GREATER_EQUAL 
| LESS_EQUAL 
| EQUAL_EQUAL 
| BANG_EQUAL 
| PLUS_EQUAL 
| MINUS_EQUAL 
| STAR_EQUAL 
| SLASH_EQUAL 
| PERCENT_EQUAL 
| RETURN 
| BREAK 
| CONTINUE 
| FN 

%%

void Bird::Parser::error( const location_type &loc, const std::string &err_message )
{
   std::cerr << "Error: " << err_message << " at line " << loc << "\n";
   /* std::cerr << "[Tried to parse token " << yylloc.token_kind_type << "]\n"; */
   /* std::cout << "Error with parsing " << std::endl; */
}