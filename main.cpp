#include <iostream>
#include <memory>

#include "lexer.h"
#include "parser.h"
#include "ast_printer.h"
#include "interpreter.h"
#include "expr.h"

int main()
{
    Interpreter interpreter;
    std::string code;
    while (true)
    {
        std::cout << ">";
        std::getline(std::cin, code);

        Lexer lexer(code);
        auto tokens = lexer.lex();
        lexer.print_tokens();

        Parser parser(tokens);
        auto ast = parser.parse();

        AstPrinter printer;
        printer.print_ast(&ast);

        interpreter.evaluate(&ast);
    }

    return 0;
}