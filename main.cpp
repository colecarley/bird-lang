#include <iostream>
#include <fstream>
#include <memory>

#include "lexer.h"
#include "parser.h"
#include "ast_printer.h"
#include "interpreter.h"
#include "expr.h"
#include "code_gen.h"

void repl();
void compile(std::string filename);

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        repl();
    }
    else
    {
        std::string filename = argv[1];
        compile(filename);
    }

    return 0;
}

void repl()
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
}

void compile(std::string filename)
{

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

    std::cout << code << std::endl;

    Lexer lexer(code);
    auto tokens = lexer.lex();
    lexer.print_tokens();

    Parser parser(tokens);
    auto ast = parser.parse();

    AstPrinter printer;
    printer.print_ast(&ast);

    CodeGen code_gen;
    code_gen.generate(&ast);
}