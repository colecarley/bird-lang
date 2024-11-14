%{
#include <string>

#include "../src/scanner.hpp"
// #include "parser.tab.hh"

#undef YY_DECL
#define YY_DECL int Bird::Scanner::yylex(Bird::Parser::semantic_type * const lval, Bird::Parser::location_type *loc )

using token = Bird::Parser::token;

#define yyterminate() return token::END

#define YY_NO_UNISTD_H

#define YY_USER_ACTION loc->step(); loc->columns(yyleng);

%}

%option debug
%option nodefault
%option noyywrap
%option yyclass = "Bird::Scanner"
%option c++

%%

%{
    yylval = lval;
%}

"+"         { std::cout << "PLUS seen" << std::endl; }
"-"         { std::cout << "PLUS seen" << std::endl; }
"*"         { std::cout << "PLUS seen" << std::endl; }
"/"         { std::cout << "PLUS seen" << std::endl; }

[0-9]+      { std::cout << "PLUS seen" << std::endl; }

%%