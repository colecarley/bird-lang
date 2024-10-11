// including in the .cpp due to a ciruclar dependency
#include "../include/parser.h"
#include "../include/ast_node/expr/binary.h"
#include "../include/ast_node/expr/unary.h"
#include "../include/ast_node/expr/primary.h"
#include "../include/ast_node/expr/ternary.h"
#include "../include/ast_node/expr/call.h"

#include "../include/ast_node/stmt/decl_stmt.h"
#include "../include/ast_node/expr/assign_expr.h"
#include "../include/ast_node/stmt/print_stmt.h"
#include "../include/ast_node/stmt/if_stmt.h"
#include "../include/ast_node/stmt/expr_stmt.h"
#include "../include/ast_node/stmt/const_stmt.h"
#include "../include/ast_node/stmt/while_stmt.h"
#include "../include/ast_node/stmt/for_stmt.h"
#include "../include/ast_node/stmt/return_stmt.h"
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
    case Token::Type::IDENTIFIER:
        if (this->is_at_end())
        {
            break;
        }
        break;
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
    case Token::Type::FOR:
        return this->for_stmt();
    case Token::Type::RETURN:
        return this->return_stmt();
    default:
        break;
    }

    return this->expr_stmt();
}

std::unique_ptr<Stmt> Parser::return_stmt()
{
    if (this->advance().token_type != Token::Type::RETURN)
    {
        throw BirdException("Expected 'return' at the beginning of return stmt");
    }

    if (this->peek().token_type == Token::Type::SEMICOLON)
    {
        this->advance();
        return std::make_unique<ReturnStmt>(ReturnStmt());
    }

    auto expr = this->expr();

    if (this->advance().token_type != Token::Type::SEMICOLON)
    {
        this->user_error_tracker->expected(";", "after const statement", this->peek());
        this->synchronize();
        throw UserException();
    }

    return std::make_unique<ReturnStmt>(ReturnStmt(std::move(expr)));
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

    std::optional<Token> type_identifier = std::nullopt;
    if (this->peek().token_type == Token::Type::COLON)
    {
        this->advance();
        if (this->peek().token_type == Token::Type::TYPE_IDENTIFIER)
        {
            type_identifier = std::make_optional<Token>(this->advance());
        }
        else
        {
            this->user_error_tracker->expected("type identifier", "after : in assignment", this->peek());
            this->synchronize();
            throw UserException();
        }
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
        this->user_error_tracker->expected("}", "at the end of block", this->peek_previous());
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
        this->user_error_tracker->expected(";", "at the end of expression", this->peek_previous());
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

std::unique_ptr<Stmt> Parser::for_stmt()
{
    if (this->advance().token_type != Token::Type::FOR)
    {
        throw BirdException("expected 'for' at the beginning of for statement");
    }

    std::optional<std::unique_ptr<Stmt>> initializer;
    if (this->peek().token_type != Token::Type::SEMICOLON)
    {
        initializer = this->stmt();
    }

    if (this->peek().token_type == Token::Type::SEMICOLON)
    {
        this->advance();
    }

    std::optional<std::unique_ptr<Expr>> condition;
    if (this->peek().token_type != Token::Type::SEMICOLON)
    {
        condition = this->expr();
    }

    if (this->peek().token_type == Token::Type::SEMICOLON)
    {
        this->advance();
    }

    std::optional<std::unique_ptr<Expr>> increment;
    if (this->peek().token_type != Token::Type::DO)
    {
        increment = this->expr();
    }

    if (this->advance().token_type != Token::Type::DO)
    {
        throw BirdException("expected 'do' at the end of for statement clauses");
    }

    auto body = this->stmt();

    return std::make_unique<ForStmt>(
        std::move(initializer),
        std::move(condition),
        std::move(increment),
        std::move(body));
}

std::unique_ptr<Stmt> Parser::var_decl()
{
    if (this->advance().token_type != Token::Type::VAR)
    {
        throw BirdException("Expected 'var' keyword");
    }

    auto identifier = this->advance(); // TODO: check that this is an identifier

    std::optional<Token> type_identifier = std::nullopt;
    if (this->peek().token_type == Token::Type::COLON)
    {
        this->advance();
        if (this->peek().token_type == Token::Type::TYPE_IDENTIFIER)
        {
            type_identifier = std::make_optional<Token>(this->advance());
        }
        else
        {
            this->user_error_tracker->expected("type identifier", "after : in assignment", this->peek());
            this->synchronize();
            throw UserException();
        }
    }

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

/*
 * might not be needed with the refactor since expr() handles precedence?
 * could be wrong though!
 */
std::unique_ptr<Expr> Parser::assign_expr()
{
    if (this->peek().token_type != Token::Type::IDENTIFIER)
    {
        throw BirdException("Expected variable identifier");
    }

    auto identifier = this->advance();

    switch (this->peek().token_type)
    {
    case Token::Type::EQUAL:
    case Token::Type::PLUS_EQUAL:
    case Token::Type::MINUS_EQUAL:
    case Token::Type::STAR_EQUAL:
    case Token::Type::SLASH_EQUAL:
    case Token::Type::PERCENT_EQUAL:
        break;
    default:
    {
        this->user_error_tracker->expected("assignment operator", "in assignment", this->peek_previous());
        this->synchronize();
        throw UserException();
    }
    }

    auto assign_operator = this->advance();

    auto assign_expr = std::make_unique<AssignExpr>(AssignExpr(identifier, assign_operator, this->expr()));

    if (this->advance().token_type != Token::Type::SEMICOLON)
    {
        this->user_error_tracker->expected(";", "at the end of expression", this->peek_previous());
        this->synchronize();
        throw UserException();
    }

    return assign_expr;
}

std::unique_ptr<Expr> Parser::expr()
{
    // ternary still has highest precedence? since it can recurce to a Primary
    auto left = this->ternary();

    // if next token is an assignment operator
    if (this->peek().token_type == Token::Type::EQUAL ||
        this->peek().token_type == Token::Type::PLUS_EQUAL ||
        this->peek().token_type == Token::Type::MINUS_EQUAL ||
        this->peek().token_type == Token::Type::STAR_EQUAL ||
        this->peek().token_type == Token::Type::SLASH_EQUAL ||
        this->peek().token_type == Token::Type::PERCENT_EQUAL)
    {
        // save token
        Token assign_operator = this->advance();

        // check that left is an identifier, if not throw BirdException
        if (auto *identifier = dynamic_cast<Primary *>(left.get()))
        {
            // despite knowing its Primary, still need to verify identifier
            if (identifier->value.token_type != Token::Type::IDENTIFIER)
            {
                throw BirdException("can not assign value to non-identifier");
            }

            auto right = this->expr(); // parse expression

            // create assignment expression with identifier, operator and expression
            return std::make_unique<AssignExpr>(identifier->value, assign_operator, std::move(right));
        }
        else
        {
            // wasnt primary, something must have gone terribly wrong if we get here
            throw BirdException("can not assign value to non-identifier");
        }
    }

    return left;
}

std::unique_ptr<Expr> Parser::ternary()
{
    auto condition = this->equality();

    if (this->peek().token_type == Token::Type::QUESTION)
    {
        this->advance();

        auto true_expr = this->expr();

        if (this->advance().token_type != Token::Type::COLON)
        {
            this->user_error_tracker->expected(":", "after true expression in ternary operator", this->peek_previous());
            this->synchronize();
            throw UserException();
        }

        auto false_expr = this->expr();

        return std::make_unique<Ternary>(
            std::move(condition),
            std::move(true_expr),
            std::move(false_expr));
    }

    return condition;
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

    return this->call();
}

std::unique_ptr<Expr> Parser::call()
{
    auto identifier = this->primary();

    if (auto primary = dynamic_cast<Primary *>(identifier.get()))
    {
        if (this->peek().token_type != Token::Type::LPAREN)
            return identifier;

        auto args = this->args();

        return std::make_unique<Call>(Call(primary->value, std::move(args)));
    }
    else
        return identifier;
}

std::vector<std::unique_ptr<Expr>> Parser::args()
{
    this->advance(); // advance past '('
    std::vector<std::unique_ptr<Expr>> args;

    while (this->peek().token_type != Token::Type::RPAREN)
    {
        args.push_back(this->expr());

        if (this->peek().token_type == Token::Type::COMMA)
            this->advance();
    }

    if (this->advance().token_type != Token::Type::RPAREN)
    {
        this->user_error_tracker->expected(")", "after function call", this->peek());
        this->synchronize();
        throw UserException();
    }

    return args;
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

std::unique_ptr<Stmt> Parser::func()
{
    if (this->advance().token_type != Token::Type::FN)
    {
        throw BirdException("expected fn keyword");
    }

    auto fn_identifier = this->advance();

    if (fn_identifier.token_type != Token::Type::IDENTIFIER)
    {
        this->user_error_tracker->expected("identifier", "after keyword", this->peek_previous());
        this->synchronize();
        throw UserException();
    }
    if (this->advance().token_type != Token::Type::LPAREN)
    {
        this->user_error_tracker->expected("(", "after identifier", this->peek_previous());
        this->synchronize();
        throw UserException();
    }

    auto fn_params = this->fn_params();

    if (this->advance().token_type != Token::Type::RPAREN)
    {
        this->user_error_tracker->expected(")", "after function parameter list", this->peek_previous());
        this->synchronize();
        throw UserException();
    }

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
        else if (this->advance().token_type != Token::Type::COMMA)
        {
            this->user_error_tracker->expected(",", "after function parameter", this->peek_previous());
            this->synchronize();
            throw UserException();
        }
    }
}

std::pair<Token, Token> Parser::param_decl()
{
    auto identifier = this->advance();

    if (identifier.token_type != Token::Type::IDENTIFIER)
    {
        this->user_error_tracker->expected("identifier", "in function parameter list", this->peek_previous());
        this->synchronize();
        throw UserException();
    }

    if (this->advance().token_type != Token::Type::COLON)
    {
        this->user_error_tracker->expected(":", "after identifier in parameter declaration", this->peek_previous());
        this->synchronize();
        throw UserException();
    }

    auto type_identifier = this->advance();

    if (type_identifier.token_type != Token::Type::TYPE_IDENTIFIER)
    {
        this->user_error_tracker->expected("type identifier", "after \':\' in parameter declaration", this->peek_previous());
        this->synchronize();
        throw UserException();
    }

    return {identifier, type_identifier};
}

std::optional<Token> Parser::fn_return_type()
{
    if (this->peek().token_type != Token::Type::ARROW)
    {
        return {};
    }

    this->advance();

    auto return_type = this->advance();

    if (return_type.token_type != Token::Type::TYPE_IDENTIFIER)
    {
        this->user_error_tracker->expected("return type", "after arrow", this->peek_previous());
        this->synchronize();
        throw UserException();
    }

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

Token Parser::peek_next()
{
    return this->tokens[this->position + 1];
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