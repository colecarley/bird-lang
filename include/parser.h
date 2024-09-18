#pragma once

#include <vector>
#include <memory>

#include "lexer.h"

// foward declarations
class Expr;
class Stmt;
class UserErrorTracker;

/*
 * Parser will take the tokens as input
 * and, using the grammar, create an AST
 * (Abstract Syntax Tree) and report as many
 * errors as it can find
 */
class Parser
{
public:
    const std::vector<Token> tokens;
    unsigned int position;
    UserErrorTracker *user_error_tracker;

    Parser(std::vector<Token> tokens, UserErrorTracker *user_error_tracker);

    std::vector<std::unique_ptr<Stmt>> parse();

    // strs
    std::unique_ptr<Stmt> stmt();

    std::unique_ptr<Stmt> expr_stmt();

    std::unique_ptr<Stmt> var_decl();

    std::unique_ptr<Stmt> const_decl();

    std::unique_ptr<Stmt> block();

    std::unique_ptr<Stmt> if_stmt();

    std::unique_ptr<Stmt> print_stmt();

    std::unique_ptr<Stmt> while_stmt();

    // exprs
    std::unique_ptr<Expr> expr();

    std::unique_ptr<Expr> equality();

    std::unique_ptr<Expr> comparison();

    std::unique_ptr<Expr> term();

    std::unique_ptr<Expr> factor();

    std::unique_ptr<Expr> unary();

    std::unique_ptr<Expr> primary();

    std::unique_ptr<Expr> grouping();

    Token advance();

    Token peek();

    Token peek_previous();

    bool is_at_end();

    void synchronize();
};
