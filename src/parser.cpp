// including in the .cpp due to a ciruclar dependency
#include "../include/parser.h"
#include "../include/ast_node/expr/binary.h"
#include "../include/ast_node/expr/unary.h"
#include "../include/ast_node/expr/primary.h"

#include "../include/ast_node/stmt/decl_stmt.h"
#include "../include/ast_node/stmt/print_stmt.h"
#include "../include/ast_node/stmt/if_stmt.h"
#include "../include/ast_node/stmt/expr_stmt.h"
#include "../include/ast_node/stmt/const_stmt.h"
#include "../include/ast_node/stmt/while_stmt.h"
#include "../include/ast_node/stmt/block.h"
#include "../include/ast_node/stmt/func.h"

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
    switch (this->peek().token_type)
    {
    case Token::Type::VAR:
        return this->var_decl();
    case Token::Type::IF:
        return this->if_stmt();
    case Token::Type::CONST:
        return this->const_decl();
    case Token::Type::PRINT:
        return this->print_stmt();
    case Token::Type::LBRACE:
        return this->block();
    case Token::Type::FN:
        return this->func();
    case Token::Type::WHILE:
        return this->while_stmt();
    default:
        return this->expr_stmt();
    }
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
        this->user_error_tracker->expected("identifier after const", this->peek());
        this->synchronize();
        throw UserException();
    }

    if (this->advance().token_type != Token::Type::COLON)
    {
        this->user_error_tracker->expected(": after identifier", this->peek());
        this->synchronize();
        throw UserException();
    }

    auto type_identifier = this->advance();

    if (type_identifier.token_type != Token::Type::TYPE_IDENTIFIER)
    {
        this->user_error_tracker->expected("type identifier after identifier", this->peek());
        this->synchronize();
        throw UserException();
    }

    if (this->advance().token_type != Token::Type::EQUAL)
    {
        this->user_error_tracker->expected("= after type identifier", this->peek());
        this->synchronize();
        throw UserException();
    }

    auto expr = this->expr();

    if (this->advance().token_type != Token::Type::SEMICOLON)
    {
        this->user_error_tracker->expected("; after const statement", this->peek());
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
        this->user_error_tracker->expected("} at the end of block", this->peek_previous());
        this->synchronize();
        throw UserException();
    }

    return std::make_unique<Block>(Block(std::move(stmts)));
}

std::unique_ptr<Stmt> Parser::if_stmt()
{
    if (this->advance().token_type != Token::Type::IF)
    {
        throw BirdException("Expected 'if' at the beginning of if statement");
    }

    auto condition = this->expr();

    auto statement = this->stmt();

    if (this->peek().token_type == Token::Type::ELSE)
    {
        this->advance();
        return std::make_unique<IfStmt>(
            std::move(condition),
            std::move(statement),
            std::make_optional<std::unique_ptr<Stmt>>(std::move(this->stmt())));
    }

    return std::make_unique<IfStmt>(
        std::move(condition),
        std::move(statement),
        std::nullopt);
}

std::unique_ptr<Stmt> Parser::expr_stmt()
{
    auto result = std::make_unique<ExprStmt>(
        ExprStmt(this->expr()));

    if (this->advance().token_type != Token::Type::SEMICOLON)
    {
        this->user_error_tracker->expected("; at the end of expression", this->peek_previous());
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
        this->user_error_tracker->expected("; after 'print'", this->peek_previous());
        this->synchronize();
        throw UserException();
    }

    auto result = std::make_unique<PrintStmt>(PrintStmt(std::move(values)));
    return result;
}

std::unique_ptr<Stmt> Parser::while_stmt()
{
    if (this->advance().token_type != Token::Type::WHILE)
    {
        throw BirdException("Expected 'while' at the beginning of while statement");
    }

    auto condition = this->expr();

    auto stmt = this->stmt();

    return std::make_unique<WhileStmt>(WhileStmt(std::move(condition), std::move(stmt)));
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
        this->user_error_tracker->expected(": after identifier in assignment", this->peek_previous());
        this->synchronize();
        throw UserException();
    }

    auto type_identifier = this->advance(); // TODO: check that this is a type identifier

    if (this->advance().token_type != Token::Type::EQUAL)
    {
        this->user_error_tracker->expected("= in assignment", this->peek_previous());
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
        this->user_error_tracker->expected("; at the end of expression", this->peek_previous());
        this->synchronize();
        throw UserException();
    }

    return result;
}

std::unique_ptr<Expr> Parser::expr()
{
    return this->equality();
}

std::unique_ptr<Expr> Parser::equality()
{
    auto left = this->comparison();

    while (this->peek().token_type == Token::Type::EQUAL_EQUAL ||
           this->peek().token_type == Token::Type::BANG_EQUAL)
    {
        Token op = this->advance();
        std::unique_ptr<Expr> right = this->comparison();
        left = std::make_unique<Binary>(Binary(std::move(left), op, std::move(right)));
    }

    return left;
}

std::unique_ptr<Expr> Parser::comparison()
{
    auto left = this->term();

    while (this->peek().token_type == Token::Type::GREATER ||
           this->peek().token_type == Token::Type::GREATER_EQUAL ||
           this->peek().token_type == Token::Type::LESS ||
           this->peek().token_type == Token::Type::LESS_EQUAL)
    {
        Token op = this->advance();
        std::unique_ptr<Expr> right = this->term();
        left = std::make_unique<Binary>(Binary(std::move(left), op, std::move(right)));
    }

    return left;
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

    while (this->peek().token_type == Token::Type::STAR ||
           this->peek().token_type == Token::Type::SLASH ||
           this->peek().token_type == Token::Type::PERCENT)
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
        this->user_error_tracker->expected("identifier or i32", this->peek());
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
        this->user_error_tracker->expected("( after grouping", this->peek_previous());
        this->synchronize();
        throw UserException();
    }

    return expr;
}

std::unique_ptr<Stmt> Parser::func()
{
    // if (this->advance().token_type != Token::Type::FN)
    // {
    //     throw BirdException("expected fn keyword");
    // }

    this->expect_token(Token::Type::FN).adv_or_bird_error("expected fn keyword");

    // auto fn_identifier = this->advance();

    // if (fn_identifier.token_type != Token::Type::IDENTIFIER)
    // {
    //     this->user_error_tracker->expected("identifier", "after keyword", this->peek_previous());
    //     this->synchronize();
    //     throw UserException();
    // }

    auto fn_identifier = this->expect_token(Token::Type::IDENTIFIER).adv_or_user_error("identifier after keyword");

    // if (this->advance().token_type != Token::Type::LPAREN)
    // {
    //     this->user_error_tracker->expected("(", "after identifier", this->peek_previous());
    //     this->synchronize();
    //     throw UserException();
    // }

    this->expect_token(Token::Type::LPAREN).adv_or_user_error("( after identifier");

    auto fn_params = this->fn_params();

    // if (this->advance().token_type != Token::Type::RPAREN)
    // {
    //     this->user_error_tracker->expected(")", "after function parameter list", this->peek_previous());
    //     this->synchronize();
    //     throw UserException();
    // }

    this->expect_token(Token::Type::RPAREN).adv_or_user_error(") after function parameter list");

    auto fn_return_type = this->fn_return_type();

    auto block = this->block();

    return std::make_unique<Func>(Func(fn_identifier, fn_return_type, fn_params, std::move(block)));
}

std::vector<std::pair<Token, Token>> Parser::fn_params()
{
    if (this->peek().token_type == Token::Type::RPAREN)
    {
        return {};
    }

    std::vector<std::pair<Token, Token>> params;

    while (true)
    {
        params.push_back(this->param_decl());

        if (this->peek().token_type == Token::Type::RPAREN)
        {
            return params;
        }
        // else if (this->advance().token_type != Token::Type::COMMA)
        // {
        //     this->user_error_tracker->expected(",", "after function parameter", this->peek_previous());
        //     this->synchronize();
        //     throw UserException();
        // }
        else
        {
            this->expect_token(Token::Type::COMMA).adv_or_user_error(", after function parameter");
        }
    }
}

std::pair<Token, Token> Parser::param_decl()
{
    // auto identifier = this->advance();

    // if (identifier.token_type != Token::Type::IDENTIFIER)
    // {
    //     this->user_error_tracker->expected("identifier", "in function parameter list", this->peek_previous());
    //     this->synchronize();
    //     throw UserException();
    // }

    auto identifier = this->expect_token(Token::Type::IDENTIFIER).adv_or_user_error("identifier in function parameter list");

    // if (this->advance().token_type != Token::Type::COLON)
    // {
    //     this->user_error_tracker->expected(":", "after identifier in parameter declaration", this->peek_previous());
    //     this->synchronize();
    //     throw UserException();
    // }

    this->expect_token(Token::Type::COLON).adv_or_user_error(": after identifier in parameter declaration");

    // auto type_identifier = this->advance();

    // if (type_identifier.token_type != Token::Type::TYPE_IDENTIFIER)
    // {
    //     this->user_error_tracker->expected("type identifier", "after \':\' in parameter declaration", this->peek_previous());
    //     this->synchronize();
    //     throw UserException();
    // }

    auto type_identifier = this->expect_token(Token::Type::TYPE_IDENTIFIER).adv_or_user_error("type identifier after \':\' in parameter declaration");

    return {identifier, type_identifier};
}

std::optional<Token> Parser::fn_return_type()
{
    if (this->peek().token_type != Token::Type::ARROW)
    {
        return {};
    }

    this->advance();

    // auto return_type = this->advance();

    // if (return_type.token_type != Token::Type::TYPE_IDENTIFIER)
    // {
    //     this->user_error_tracker->expected("return type", "after arrow", this->peek_previous());
    //     this->synchronize();
    //     throw UserException();
    // }

    auto return_type = this->expect_token(Token::Type::TYPE_IDENTIFIER).adv_or_user_error("return type after arrow");

    return return_type;
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

Parser::ParseOption Parser::expect_token(Token::Type type)
{
    auto token = this->peek();
    if (token.token_type == type)
    {
        return ParseOption(token, *this);
    }
    else
    {
        return ParseOption(*this);
    }
}

Token Parser::ParseOption::adv_or_user_error(std::string context)
{
    if (token)
    {
        parser.advance();
        return token.value();
    }
    else
    {
        parser.user_error_tracker->expected(context, parser.peek());
        parser.synchronize();
        throw UserException();
    }
}

Token Parser::ParseOption::adv_or_bird_error(std::string message)
{
    if (token)
    {
        parser.advance();
        return token.value();
    }
    else
    {
        throw BirdException(message);
    }
}