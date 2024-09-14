// including in the .cpp due to a ciruclar dependency
#include "../include/parser.h"
#include "../include/ast_node/expr/binary.h"
#include "../include/ast_node/expr/unary.h"
#include "../include/ast_node/expr/primary.h"

#include "../include/ast_node/stmt/decl_stmt.h"
#include "../include/ast_node/stmt/print_stmt.h"
#include "../include/ast_node/stmt/expr_stmt.h"
#include "../include/ast_node/stmt/const_stmt.h"
#include "../include/ast_node/stmt/block.h"

#include "../include/exceptions/bird_exception.h"
#include "../include/exceptions/user_exception.h"
#include "../include/exceptions/user_error_tracker.h"

Parser::Parser(std::vector<Token> tokens, UserErrorTracker *user_error_tracker) : tokens(tokens)
{
    this->position = 0;
    this->user_error_tracker = user_error_tracker;
}

std::vector<std::unique_ptr<Stmt>> Parser::parse()
{
    std::vector<std::unique_ptr<Stmt>> stmts;
    while (!this->is_at_end())
    {
        try
        {
            auto stmt = this->stmt();
            stmts.push_back(std::move(stmt));
        }
        catch (UserException error)
        {
            // do nothing
        }
    }

    return stmts;
}

std::unique_ptr<Stmt> Parser::stmt()
{
    if (this->peek().token_type == Token::Type::VAR)
    {
        return this->var_decl();
    }
    if (this->peek().token_type == Token::Type::CONST)
    {
        return this->const_decl();
    }
    if (this->peek().token_type == Token::Type::PRINT)
    {
        return this->print_stmt();
    }
    if (this->peek().token_type == Token::Type::LBRACE)
    {
        return this->block();
    }

    return this->expr_stmt();
}

std::unique_ptr<Stmt> Parser::const_decl()
{
    if (this->advance().token_type != Token::Type::CONST)
    {
        throw BirdException("Expected 'const' at the beginning of const decl");
    }

    auto identifier = this->advance();

    if (identifier.token_type != Token::Type::IDENTIFIER)
    {
        this->user_error_tracker->expected("identifier", "after const", this->peek());
        this->synchronize();
        throw UserException();
    }

    if (this->advance().token_type != Token::Type::COLON)
    {
        this->user_error_tracker->expected(":", "after identifier", this->peek());
        this->synchronize();
        throw UserException();
    }

    auto type_identifier = this->advance();

    if (type_identifier.token_type != Token::Type::TYPE_IDENTIIFER)
    {
        this->user_error_tracker->expected("type identifier", "after identifier", this->peek());
        this->synchronize();
        throw UserException();
    }

    if (this->advance().token_type != Token::Type::EQUAL)
    {
        this->user_error_tracker->expected("=", "after type identifier", this->peek());
        this->synchronize();
        throw UserException();
    }

    auto expr = this->expr();

    if (this->advance().token_type != Token::Type::SEMICOLON)
    {
        this->user_error_tracker->expected(";", "after const statement", this->peek());
        this->synchronize();
        throw UserException();
    }

    return std::make_unique<ConstStmt>(
        ConstStmt(
            identifier, type_identifier, std::move(expr)));
}

std::unique_ptr<Stmt> Parser::block()
{
    if (this->advance().token_type != Token::Type::LBRACE)
    {
        throw BirdException("Expected '{' at the beginning of block");
    }

    auto stmts = std::vector<std::unique_ptr<Stmt>>();

    while (this->peek().token_type != Token::Type::RBRACE && !this->is_at_end())
    {
        auto stmt = this->stmt();
        stmts.push_back(std::move(stmt));
    }

    if (this->advance().token_type != Token::Type::RBRACE)
    {
        this->user_error_tracker->expected("{", "at the end of block", this->peek_previous());
        this->synchronize();
        throw UserException();
    }

    return std::make_unique<Block>(Block(std::move(stmts)));
}

std::unique_ptr<Stmt> Parser::expr_stmt()
{
    auto result = std::make_unique<ExprStmt>(
        ExprStmt(this->expr()));

    if (this->advance().token_type != Token::Type::SEMICOLON)
    {
        this->user_error_tracker->expected("{", "at the end of expression", this->peek_previous());
        this->synchronize();
        throw UserException();
    }

    return result;
}

std::unique_ptr<Stmt> Parser::print_stmt()
{
    if (this->advance().token_type != Token::Type::PRINT)
    {
        throw BirdException("Expected 'print' keyword");
    }

    auto values = std::vector<std::unique_ptr<Expr>>();
    while (this->peek().token_type != Token::Type::SEMICOLON)
    {
        auto expr = this->expr();
        values.push_back(std::move(expr));

        if (this->peek().token_type != Token::Type::COMMA)
        {
            break;
        }

        this->advance();
    }

    if (this->advance().token_type != Token::Type::SEMICOLON)
    {
        this->user_error_tracker->expected(";", "after 'print'", this->peek_previous());
        this->synchronize();
        throw UserException();
    }

    auto result = std::make_unique<PrintStmt>(PrintStmt(std::move(values)));
    return result;
}

std::unique_ptr<Stmt> Parser::var_decl()
{
    if (this->advance().token_type != Token::Type::VAR)
    {
        throw BirdException("Expected 'var' keyword");
    }

    auto identifier = this->advance(); // TODO: check that this is an identifier

    if (this->advance().token_type != Token::Type::COLON)
    {
        this->user_error_tracker->expected(":", "after identifier in assignment", this->peek_previous());
        this->synchronize();
        throw UserException();
    }

    auto type_identifier = this->advance(); // TODO: check that this is a type identifier

    if (this->advance().token_type != Token::Type::EQUAL)
    {
        this->user_error_tracker->expected("=", "in assignment", this->peek_previous());
        this->synchronize();
        throw UserException();
    }

    auto result = std::make_unique<DeclStmt>(
        DeclStmt(
            identifier,
            type_identifier,
            this->expr()));

    if (this->advance().token_type != Token::Type::SEMICOLON)
    {
        this->user_error_tracker->expected(";", "at the end of expression", this->peek_previous());
        this->synchronize();
        throw UserException();
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

    while (this->peek().token_type == Token::Type::PLUS || this->peek().token_type == Token::Type::MINUS)
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

    while (this->peek().token_type == Token::Type::STAR || this->peek().token_type == Token::Type::SLASH)
    {
        Token op = this->advance();
        auto right = this->unary();
        left = std::make_unique<Binary>(Binary(std::move(left), op, std::move(right)));
    }

    return left;
}

std::unique_ptr<Expr> Parser::unary()
{
    if (this->peek().token_type == Token::Type::MINUS)
    {
        Token op = this->advance();
        std::unique_ptr<Expr> expr = this->unary();
        return std::make_unique<Unary>(Unary(op, std::move(expr)));
    }

    return this->primary();
}

std::unique_ptr<Expr> Parser::primary()
{
    Token::Type token_type = this->peek().token_type;

    switch (token_type)
    {
    case Token::Type::IDENTIFIER:
    case Token::Type::INT_LITERAL:
    case Token::Type::FLOAT_LITERAL:
    case Token::Type::STR_LITERAL:
    case Token::Type::BOOL_LITERAL:
        return std::make_unique<Primary>(Primary(this->advance()));
    case Token::Type::LPAREN:
        return this->grouping();
    default:
    {
        this->user_error_tracker->expected("identifier or i32", "", this->peek());
        this->synchronize();
        throw UserException();
    }
    }
}

std::unique_ptr<Expr> Parser::grouping()
{
    if (this->advance().token_type != Token::Type::LPAREN)
    {
        throw BirdException("Expected '(' before grouping");
    }

    auto expr = this->expr();

    if (this->advance().token_type != Token::Type::RPAREN)
    {
        this->user_error_tracker->expected("(", "after grouping", this->peek_previous());
        this->synchronize();
        throw UserException();
    }

    return expr;
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

Token Parser::peek_previous()
{
    return this->tokens[this->position - 1];
}

bool Parser::is_at_end()
{
    return this->peek().token_type == Token::Type::END;
}

void Parser::synchronize()
{
    while (this->advance().token_type != Token::Type::SEMICOLON && !this->is_at_end())
    {
        // do nothing
    }
}