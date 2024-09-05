#include <iostream>
#include <fstream>
#include <memory>

#include "lexer.h"
#include "parser.h"
#include "ast_printer.h"
#include "interpreter.h"
#include "expr.h"

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        throw BirdException("Expected filename");
    }

    std::string filename = argv[1];
    std::ifstream file(filename);
    std::string code;
    if (file.is_open())
    {
        std::string line;
        while (file.good())
        {
            getline(file, line);
            code += line;
        }
    }

    Lexer lexer(code);
    auto tokens = lexer.lex();
    lexer.print_tokens();

    Parser parser(tokens);
    auto ast = parser.parse();

    AstPrinter printer;
    printer.print_ast(&ast);

    Interpreter interpreter;
    interpreter.evaluate(&ast);

    return 0;
}
