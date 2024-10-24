#include <iostream>
#include <fstream>
#include <memory>

#include "include/lexer.h"
#include "include/parser.h"
#include "include/visitors/ast_printer.h"
#include "include/visitors/interpreter.h"
#include "include/visitors/type_checker.h"

#include "include/ast_node/expr/expr.h"
#include "include/visitors/code_gen.h"
#include "include/exceptions/user_error_tracker.h"

void repl();
void compile(std::string filename);
void interpret(std::string filename);
std::string read_file(std::string filename);

int main(int argc, char *argv[])
{
    if (argc == 1)
    {
        repl();
    }
    else if (argc == 2)
    {
        std::string filename = argv[1];
        compile(filename);
    }
    else
    {
        if (!strcmp(argv[1], "-i"))
        {
            interpret(std::string(argv[2]));
        }
        else if (!strcmp(argv[2], "-i"))
        {
            interpret(std::string(argv[1]));
        }
    }

    return 0;
}

void repl()
{
    Interpreter interpreter;
    TypeChecker type_checker;
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
            type_checker.check_types(&ast);
        }
        catch (std::exception e)
        {
            std::cout << "type error: " << e.what() << std::endl;
        }

        try
        {
            interpreter.evaluate(&ast);
        }
        catch (BirdException e)
        {
            std::cout << e.what() << std::endl;
        }
        catch (std::exception e)
        {
            std::cout << "err" << std::endl;
        }
    }
}

void compile(std::string filename)
{
    auto code = read_file(filename);
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

void interpret(std::string filename)
{
    std::cout << "filename " << filename << std::endl;
    auto code = read_file(filename);
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
    Interpreter interpreter;

    try
    {
        interpreter.evaluate(&ast);
    }
    catch (BirdException e)
    {
        std::cout << e.what() << std::endl;
    }
    catch (std::exception e)
    {
        std::cout << "err" << std::endl;
    }
}

std::string read_file(std::string filename)
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

    return code;
}