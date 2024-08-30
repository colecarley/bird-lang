#include <iostream>
#include <memory>

#include "lexer.h"
#include "parser.h"
#include "ast_printer.h"
#include "interpreter.h"
#include "expr.h"

int main()
{
    Lexer lexer("1 + 2 * 3");
    auto tokens = lexer.lex();
    lexer.print_tokens();

    Parser parser(tokens);
    std::unique_ptr<Expr> ast = parser.parse();

    AstPrinter printer;
    printer.print_ast(ast.get());

    Interpreter interpreter;
    interpreter.evaluate(ast.get());

    return 0;
}