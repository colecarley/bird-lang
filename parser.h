#pragma once

#include <vector>
#include <memory>

#include "lexer.h"

class Expr;
class Stmt;

class Parser
{
public:
    const std::vector<Token> tokens;
    unsigned int position;

    Parser(std::vector<Token> tokens);

    std::vector<std::unique_ptr<Stmt>> parse();

    std::unique_ptr<Stmt> stmt();

    std::unique_ptr<Stmt> exprStmt();

    std::unique_ptr<Stmt> varDecl();

    std::unique_ptr<Stmt> printStmt();

    std::unique_ptr<Expr> expr();

    std::unique_ptr<Expr> term();

    std::unique_ptr<Expr> factor();

    std::unique_ptr<Expr> unary();

    std::unique_ptr<Expr> primary();

    Token advance();

    Token peek();

    bool is_at_end();
};
