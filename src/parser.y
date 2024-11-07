%{
    /*
        * This is the parser specification file
    */
    #include <iostream>

    // this needs to be defined in the parser.y file
    void yyerror(const char* s);

    // this is defined by us, and is the function that will be called from the main.cpp file 
    void bird_parse(const char* code);

    // these are the functions that are defined in the scanner.l file
    extern int yylex();
    extern void scanner_destroy();
    extern void scanner_init(const char* code);
%}

%union {
    int intval;
}

%token PLUS
%token MINUS

%token <intval> INT_LITERAL;

%type <intval> expr term primary 

%%
expr:
    term { $$ = $1; std::cout << "result: " << $$ << std::endl; /* TODO: remove this */ }
    ;

term:
    primary PLUS term { $$ = $1 + $3; }
    | primary MINUS term { $$ = $1 - $3; }
    | primary
    ;

primary:
    INT_LITERAL { $$ = $1; }
    ;

%%

void yyerror(const char *s) {
    std::cerr << "Error: " << s << std::endl;
}

void bird_parse(const char* code) {
    scanner_init(code);
    yyparse();
    scanner_destroy();
}
