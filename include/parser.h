#pragma once

#include <vector>
#include <memory>

#include "lexer.h"

// foward declarations
class Expr;
class Stmt;
class Func;
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

    struct ExpectedToken;

    Parser(std::vector<Token> tokens, UserErrorTracker *user_error_tracker);

    std::vector<std::unique_ptr<Stmt>> parse();

    // strs
    std::unique_ptr<Stmt> stmt();

    std::unique_ptr<Stmt> expr_stmt();

    std::unique_ptr<Stmt> var_decl();

    std::unique_ptr<Stmt> const_decl();

    std::unique_ptr<Stmt> assign_stmt();

    std::unique_ptr<Stmt> block();

    std::unique_ptr<Stmt> if_stmt();

    std::unique_ptr<Stmt> print_stmt();

    std::unique_ptr<Stmt> func();

    std::vector<std::pair<Token, Token>> fn_params();

    std::pair<Token, Token> param_decl();

    std::optional<Token> fn_return_type();

    std::unique_ptr<Stmt> while_stmt();

    // exprs
    std::unique_ptr<Expr> expr();

    std::unique_ptr<Expr> ternary();

    std::unique_ptr<Expr> equality();

    std::unique_ptr<Expr> comparison();

    std::unique_ptr<Expr> term();

    std::unique_ptr<Expr> factor();

    std::unique_ptr<Expr> unary();

    std::unique_ptr<Expr> primary();

    std::unique_ptr<Expr> grouping();

    ExpectedToken expect_token(Token::Type type);

    ExpectedToken expect_one_of(std::initializer_list<Token::Type> types);

    Token advance();

    Token peek();

    Token peek_next();

    Token peek_previous();

    bool is_at_end();

    void synchronize();

    // A type to facilitate error handling in parsing code
    // Example usage: `this->expect_token(Token::Type::IDENTIFIER).adv_or_user_error("identifier after var keyword")`
    struct ExpectedToken
    {
    private:
        std::optional<Token> token;
        Parser &parser;

        ExpectedToken(Token token, Parser &parser) : token(token), parser(parser) {}
        ExpectedToken(Parser &parser) : token(std::nullopt), parser(parser) {}
        ExpectedToken() = delete;

    public:
        friend ExpectedToken Parser::expect_token(Token::Type type);
        friend ExpectedToken Parser::expect_one_of(std::initializer_list<Token::Type> types);

        // if the parseoption contains a token, advance and return the token, otherwise throw the appropriate error
        Token adv_or_user_error(std::string context);
        Token adv_or_bird_error(std::string message);
        bool is_valid();
        bool is_invalid();
    };
};