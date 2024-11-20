%{
#include <string>

#include "../src/scanner.hpp"

using token = Bird::Parser::token;

#define yyterminate() return token::END

#define YY_NO_UNISTD_H

#define YY_USER_ACTION location->step(); location->columns(yyleng);

%}

%option debug
%option nodefault
%option noyywrap
%option yyclass = "Bird::Scanner"
%option c++

%%

%{
    // code to run at the start of the lexer
%}

"var"                     {return token:: VAR;} 
"const"                   {return token::CONST;} 
"print"                   {return token::PRINT;} 
"if"                      {return token::IF;} 
"else"                    {return token::ELSE;} 
"while"                   {return token::WHILE;} 
"for"                     {return token::FOR;} 
"do"                      {return token::DO;} 
"fn"                      {return token::FN;} 
"return"                  {return token::RETURN;} 
"break"                   {return token::BREAK;} 
"continue"                {return token::CONTINUE;} 

int|float|boolstring      {yylval.identifier = yytext;
                           return token::TYPE_IDENTIFIER;} 

[0-9]+\.[0-9]+            {yylval.double_val = atof(yytext);
                           return token::FLOAT_LITERAL;} 
[0-9]+                    {yylval.int_val = atoi(yytext);
                           return token::INT_LITERAL;} 
"true"                    {yylval.bool_val = true;
                           return token::BOOL_LITERAL;} 
"false"                   {yylval.bool_val = false;
                           return token::BOOL_LITERAL;}
\".*\"                    {yylval.string_val = yytext;
                           return token::STR_LITERAL;} 

[a-zA-z][a-zA-Z0-9\-\_]* {yylval.identifier = yytext;
                         return token::IDENTIFIER;} 

","                     {return token::COMMA;} 
"}"                     {return token::RBRACE;} 
"{"                     {return token::LBRACE;} 
")"                     {return token::RPAREN;} 
"("                     {return token::LPAREN;} 
"-"                     {return token::MINUS;} 
"%"                     {return token::PERCENT;} 
"+"                     {return token::PLUS;} 
"/"                     {return token::SLASH;} 
"*"                     {return token::STAR;} 
";"                     {return token::SEMICOLON;} 
"?"                     {return token::QUESTION;} 
"="                     {return token::EQUAL;} 
":"                     {return token::COLON;} 
">"                     {return token::GREATER;} 
">="                    {return token::GREATER_EQUAL;} 
"<"                     {return token::LESS;} 
"<="                    {return token::LESS_EQUAL;} 
"=="                    {return token::EQUAL_EQUAL;} 
"!="                    {return token::BANG_EQUAL;} 
"+="                    {return token::PLUS_EQUAL;} 
"-="                    {return token::MINUS_EQUAL;} 
"*="                    {return token::STAR_EQUAL;} 
"/="                    {return token::SLASH_EQUAL;} 
"%="                    {return token::PERCENT_EQUAL;} 
"!"                     {return token::BANG;} 
"->"                    {return token::ARROW;} 

\n                      {location->step();}

[ \t\r]*                {}

<<EOF>>     {yyterminate();}

%%