#include <iostream>
#include <fstream>
#include <memory>

#include <llvm/IR/Module.h>

#include "include/lexer.h"
#include "include/parser.h"
#include "include/visitors/ast_printer.h"
#include "include/visitors/interpreter.h"
#include "include/ast_node/expr/expr.h"
#include "include/visitors/code_gen.h"
#include "include/exceptions/user_error_tracker.h"

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

        UserErrorTracker error_tracker(code);

        Lexer lexer(code, &error_tracker);
        auto tokens = lexer.lex();
        lexer.print_tokens();

        Parser parser(tokens, &error_tracker);
        auto ast = parser.parse();

        if (error_tracker.has_errors())
        {
            error_tracker.print_errors_and_exit();
        }

        AstPrinter printer;
        printer.print_ast(&ast);

        try
        {
            interpreter.evaluate(&ast);
        }
        catch (std::exception e)
        {
            std::cout << "err" << std::endl;
        }
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
            code += line += '\n';
        }
    }

    UserErrorTracker error_tracker(code);

    Lexer lexer(code, &error_tracker);
    auto tokens = lexer.lex();
    lexer.print_tokens();

    Parser parser(tokens, &error_tracker);
    auto ast = parser.parse();

    if (error_tracker.has_errors())
    {
        error_tracker.print_errors_and_exit();
    }

    AstPrinter printer;
    printer.print_ast(&ast);

    CodeGen code_gen;
    code_gen.generate(&ast);
}