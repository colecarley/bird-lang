%skeleton "lalr1.cc"
%require  "3.2"
%debug 
%defines 
%define api.namespace {Bird}
%define api.parser.class {Parser}


%code requires{
   namespace Bird {
      class Scanner;
   }
}

%parse-param {Bird::Scanner &scanner}

%code {
   #include "../src/scanner.hpp"
   #undef yylex
   #define yylex scanner.yylex
}

%define api.value.type variant
%define parse.assert

%token               END    0     "end of file"
%token               UPPER
%token               LOWER
%token <std::string> WORD
%token               NEWLINE
%token               CHAR
%token <int>         NUMBER

%locations

%%

end: END

%%

void Bird::Parser::error( const location_type &l, const std::string &err_message )
{
   // std::cerr << "Error: " << err_message << " at " << l << "\n";
}