#include "parser.h"
#include "binary.h"
#include "unary.h"
#include "primary.h"

Parser::Parser(std::vector<Token> tokens) : tokens(tokens)
{
    this->position = 0;
}

std::unique_ptr<Expr> Parser::parse()
{
    return this->expr();
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