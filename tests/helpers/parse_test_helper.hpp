#include <vector>
#include <memory>

#include "../../include/exceptions/user_error_tracker.h"
#include "../../include/parser.h"
#include "../../include/lexer.h"
#include "../../src/lexer.cpp"
#include "../../src/parser.cpp"

#include "../../include/ast_node/expr/binary.h"
#include "../../include/ast_node/expr/primary.h"

class Stmt;

std::vector<std::unique_ptr<Stmt>> parse_code(const std::string &code)
{
    UserErrorTracker error_tracker(code);
    Lexer lexer(code, &error_tracker);
    std::vector<Token> tokens = lexer.lex();

    Parser parser(tokens, &error_tracker);
    return parser.parse();
}

void print_expr(Expr *expr)
{
    if (auto binary = dynamic_cast<Binary *>(expr))
    {
        std::cout << "(";
        print_expr(binary->left.get());
        std::cout << " " << binary->op.lexeme << " ";
        print_expr(binary->right.get());
        std::cout << ")";
    }
    else if (auto primary = dynamic_cast<Primary *>(expr))
    {
        std::cout << primary->value.lexeme;
    }
}