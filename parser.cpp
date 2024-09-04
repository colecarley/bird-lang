#include "parser.h"
#include "binary.h"
#include "unary.h"
#include "primary.h"
#include "decl_stmt.h"
#include "print_stmt.h"
#include "expr_stmt.h"
#include "bird_exception.h"

Parser::Parser(std::vector<Token> tokens) : tokens(tokens)
{
    this->position = 0;
}

std::vector<std::unique_ptr<Stmt>> Parser::parse()
{
    std::vector<std::unique_ptr<Stmt>> stmts;
    while (!this->is_at_end())
    {
        stmts.push_back(this->stmt());
    }

    return stmts;
}

std::unique_ptr<Stmt> Parser::stmt()
{
    if (this->peek().token_type == TokenType::LET)
    {
        return this->varDecl();
    }
    if (this->peek().token_type == TokenType::PUTS)
    {
        return this->printStmt();
    }

    return this->exprStmt();
}

std::unique_ptr<Stmt> Parser::exprStmt()
{
    auto result = std::make_unique<ExprStmt>(
        ExprStmt(this->expr()));

    if (this->advance().token_type != TokenType::SEMICOLON)
    {
        throw BirdException("Expected ';' at the end of expression");
    }

    return result;
}

std::unique_ptr<Stmt> Parser::printStmt()
{
    if (this->advance().token_type != TokenType::PUTS)
    {
        throw BirdException("Expected 'puts' keyword");
    }

    auto values = std::vector<std::unique_ptr<Expr>>();
    while (this->peek().token_type != TokenType::SEMICOLON)
    {
        auto expr = this->expr();
        values.push_back(std::move(expr));

        if (this->peek().token_type != TokenType::COMMA)
        {
            break;
        }

        this->advance();
    }

    if (this->advance().token_type != TokenType::SEMICOLON)
    {
        throw BirdException("Expected ';' after 'puts'");
    }

    auto result = std::make_unique<PrintStmt>(PrintStmt(std::move(values)));
    return result;
}

std::unique_ptr<Stmt> Parser::varDecl()
{
    if (this->advance().token_type != TokenType::LET)
    {
        throw BirdException("Expected 'let' keyword");
    }

    auto identifier = this->advance();

    if (this->advance().token_type != TokenType::EQUAL)
    {
        throw BirdException("Expected '=' in assignment");
    }

    auto result = std::make_unique<DeclStmt>(
        DeclStmt(
            identifier,
            this->expr()));

    if (this->advance().token_type != TokenType::SEMICOLON)
    {
        throw BirdException("Expected ';' at the end of expression");
    }

    return result;
}

std::unique_ptr<Expr> Parser::expr()
{
    return this->term();
}

std::unique_ptr<Expr> Parser::term()
{
    std::unique_ptr<Expr> left = this->factor();

    while (this->peek().token_type == TokenType::PLUS || this->peek().token_type == TokenType::MINUS)
    {
        Token op = this->advance();
        std::unique_ptr<Expr> right = this->factor();
        left = std::make_unique<Binary>(Binary(std::move(left), op, std::move(right)));
    }

    return left;
}

std::unique_ptr<Expr> Parser::factor()
{
    auto left = this->unary();

    while (this->peek().token_type == TokenType::STAR || this->peek().token_type == TokenType::SLASH)
    {
        Token op = this->advance();
        auto right = this->unary();
        left = std::make_unique<Binary>(Binary(std::move(left), op, std::move(right)));
    }

    return left;
}

std::unique_ptr<Expr> Parser::unary()
{
    if (this->peek().token_type == TokenType::MINUS)
    {
        Token op = this->advance();
        std::unique_ptr<Expr> expr = this->unary();
        return std::make_unique<Unary>(Unary(op, std::move(expr)));
    }

    return this->primary();
}

std::unique_ptr<Expr> Parser::primary()
{
    TokenType token_type = this->peek().token_type;

    if (token_type == TokenType::IDENTIFIER)
    {
        return std::make_unique<Primary>(Primary(this->advance()));
    }
    if (token_type == TokenType::I32_LITERAL)
    {
        return std::make_unique<Primary>(Primary(this->advance()));
    }

    throw BirdException("Invalid primary value, expected identifier or i32");
}

Token Parser::advance()
{
    const auto token = this->peek();
    this->position++;
    return token;
}

Token Parser::peek()
{
    return this->tokens[this->position];
}

bool Parser::is_at_end()
{
    return this->position >= this->tokens.size();
}