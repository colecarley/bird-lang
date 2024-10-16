// including in the .cpp due to a ciruclar dependency
#include "../include/parser.h"
#include "../include/ast_node/expr/binary.h"
#include "../include/ast_node/expr/unary.h"
#include "../include/ast_node/expr/primary.h"
#include "../include/ast_node/expr/ternary.h"
#include "../include/ast_node/expr/call.h"

#include "../include/ast_node/stmt/decl_stmt.h"
#include "../include/ast_node/stmt/assign_stmt.h"
#include "../include/ast_node/stmt/print_stmt.h"
#include "../include/ast_node/stmt/if_stmt.h"
#include "../include/ast_node/stmt/expr_stmt.h"
#include "../include/ast_node/stmt/const_stmt.h"
#include "../include/ast_node/stmt/while_stmt.h"
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

        switch (this->peek_next().token_type)
        {
        case Token::Type::EQUAL:
        case Token::Type::PLUS_EQUAL:
        case Token::Type::MINUS_EQUAL:
        case Token::Type::STAR_EQUAL:
        case Token::Type::SLASH_EQUAL:
        case Token::Type::PERCENT_EQUAL:
            return this->assign_stmt();
        default:
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
    case Token::Type::RETURN:
        return this->return_stmt();
    default:
        break;
    }

    return this->expr_stmt();
}

// CONST_DECL: const IDENTIFIER: TYPE_IDENTIFIER = EXPR;
std::unique_ptr<Stmt> Parser::const_decl()
{
    this->expect_token(Token::Type::CONST).adv_or_bird_error("Expected 'const' at the beginning of const decl");

    auto identifier = this->expect_token(Token::Type::IDENTIFIER).adv_or_user_error("expected identifier after const keyword");

    this->expect_token(Token::Type::COLON).adv_or_user_error("expected ':' after identifier");

    std::optional<Token> type_identifier = std::nullopt;
    if (this->expect_token(Token::Type::COLON).is_valid())
    {
        this->advance();
        type_identifier = std::make_optional<Token>(
            this->expect_token(Token::Type::TYPE_IDENTIFIER).adv_or_user_error("expected type identifier after ':' in assignment"));
    }

    this->expect_token(Token::Type::EQUAL).adv_or_user_error("expected '=' in constant declaration");

    auto expr = this->expr();

    this->expect_token(Token::Type::SEMICOLON).adv_or_user_error("expected ';' after const statement");

    return std::make_unique<ConstStmt>(
        ConstStmt(identifier, type_identifier, std::move(expr)));
}

// BLOCK: {STMT*}
std::unique_ptr<Stmt> Parser::block()
{
    this->expect_token(Token::Type::LBRACE).adv_or_bird_error("Expected '{' at the beginning of block");

    auto stmts = std::vector<std::unique_ptr<Stmt>>();

    while (this->expect_token(Token::Type::RBRACE).is_invalid())
    {
        auto stmt = this->stmt();
        stmts.push_back(std::move(stmt));
    }

    this->expect_token(Token::Type::RBRACE).adv_or_user_error("expected '}' at the end of block");

    return std::make_unique<Block>(Block(std::move(stmts)));
}

// IF_STATEMENT: if EXPRESSION STATEMENT ?(ELSE STMT)
std::unique_ptr<Stmt> Parser::if_stmt()
{
    this->expect_token(Token::Type::IF).adv_or_bird_error("Expected 'if' at the beginning of if statement");

    auto condition = this->expr();

    auto statement = this->stmt();

    if (this->expect_token(Token::Type::ELSE).is_invalid())
    {
        return std::make_unique<IfStmt>(
            std::move(condition),
            std::move(statement),
            std::nullopt);
    }

    this->expect_token(Token::Type::ELSE).adv_or_bird_error("Expected else token");
    auto else_stmt = this->stmt();

    return std::make_unique<IfStmt>(
        std::move(condition),
        std::move(statement),
        std::make_optional<std::unique_ptr<Stmt>>(std::move(this->stmt())));
}

// EXPR_STMT: EXPR;
std::unique_ptr<Stmt> Parser::expr_stmt()
{
    auto result = std::make_unique<ExprStmt>(
        ExprStmt(this->expr()));

    this->expect_token(Token::Type::SEMICOLON).adv_or_user_error("expected ';' at the end of expression");

    return result;
}

// PRINT_STMT: print (STMT,)* STMT;
std::unique_ptr<Stmt> Parser::print_stmt()
{
    this->expect_token(Token::Type::PRINT).adv_or_bird_error("Expected 'print' keyword");

    auto values = std::vector<std::unique_ptr<Expr>>();
    while (this->expect_token(Token::Type::SEMICOLON).is_invalid())
    {
        auto expr = this->expr();
        values.push_back(std::move(expr));

        if (this->expect_token(Token::Type::COMMA).is_invalid())
        {
            break;
        }

        this->advance();
    }

    this->expect_token(Token::Type::SEMICOLON).adv_or_user_error("expected ';' after print statement");

    return std::make_unique<PrintStmt>(PrintStmt(std::move(values)));
}

// WHILE_STMT: while EXPR STMT
std::unique_ptr<Stmt> Parser::while_stmt()
{
    this->expect_token(Token::Type::WHILE).adv_or_bird_error("Expected 'while' at the beginning of while statement");

    auto condition = this->expr();

    auto stmt = this->stmt();

    return std::make_unique<WhileStmt>(WhileStmt(std::move(condition), std::move(stmt)));
}

// VAR_DECL: var IDENTIFIER: TYPE_IDENTIFIER = EXPR;
std::unique_ptr<Stmt> Parser::var_decl()
{

    this->expect_token(Token::Type::VAR).adv_or_bird_error("Expected 'var' keyword");

    auto identifier = this->expect_token(Token::Type::IDENTIFIER).adv_or_user_error("expected identifier after var keyword");

    std::optional<Token> type_identifier = std::nullopt;
    if (this->expect_token(Token::Type::COLON).is_valid())
    {
        this->advance();
        type_identifier = std::make_optional<Token>(
            this->expect_token(Token::Type::TYPE_IDENTIFIER).adv_or_user_error("expected type identifier after ':' in assignment"));
    }

    this->expect_token(Token::Type::EQUAL).adv_or_user_error("expected '=' in variable declaration");

    auto result = std::make_unique<DeclStmt>(
        DeclStmt(
            identifier,
            type_identifier,
            this->expr()));

    this->expect_token(Token::Type::SEMICOLON).adv_or_user_error("expected ';' at the end of expression");

    return result;
}

std::unique_ptr<Stmt> Parser::assign_stmt()
{
    auto identifier = this->expect_token(Token::Type::IDENTIFIER).adv_or_bird_error("Expected variable identifier in assignment statement");

    auto assign_operator =
        this->expect_one_of({Token::Type::EQUAL,
                             Token::Type::PLUS_EQUAL,
                             Token::Type::MINUS_EQUAL,
                             Token::Type::STAR_EQUAL,
                             Token::Type::SLASH_EQUAL,
                             Token::Type::PERCENT_EQUAL})
            .adv_or_user_error("expected assignment operator in assignment");

    auto assign_stmt = std::make_unique<AssignStmt>(AssignStmt(identifier, assign_operator, this->expr()));

    this->expect_token(Token::Type::SEMICOLON).adv_or_user_error("expected ';' at the end of expression");

    return assign_stmt;
}

std::unique_ptr<Expr> Parser::expr()
{
    return this->ternary();
}

std::unique_ptr<Expr> Parser::ternary()
{
    auto condition = this->equality();

    if (this->expect_token(Token::Type::QUESTION).is_valid())
    {
        this->expect_token(Token::Type::QUESTION).adv_or_bird_error("Expected '?' in ternary statment");

        auto true_expr = this->expr();

        this->expect_token(Token::Type::COLON).adv_or_user_error("expected ':' after true branch in ternary expression");

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

    std::unique_ptr<Binary> equality_expr;

    while (this->expect_one_of({Token::Type::EQUAL_EQUAL, Token::Type::BANG_EQUAL}).is_valid())
    {
        Token op = this->advance();
        std::unique_ptr<Expr> right = this->comparison();
        equality_expr = std::make_unique<Binary>(Binary(std::move(left), op, std::move(right)));
    }

    return equality_expr;
}

std::unique_ptr<Expr> Parser::comparison()
{
    auto left = this->term();

    while (this->expect_one_of({Token::Type::GREATER,
                                Token::Type::GREATER_EQUAL,
                                Token::Type::LESS,
                                Token::Type::LESS_EQUAL})
               .is_valid())
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

    while (this->expect_one_of({Token::Type::PLUS, Token::Type::MINUS}).is_valid())
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

    while (this->expect_one_of({Token::Type::STAR,
                                Token::Type::SLASH,
                                Token::Type::PERCENT})
               .is_valid())
    {
        Token op = this->advance();
        auto right = this->unary();
        left = std::make_unique<Binary>(Binary(std::move(left), op, std::move(right)));
    }

    return left;
}

std::unique_ptr<Expr> Parser::unary()
{
    if (this->expect_token(Token::Type::MINUS).is_valid())
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
    this->expect_token(Token::Type::LPAREN).adv_or_bird_error("Expected '(' at start of args list");
    std::vector<std::unique_ptr<Expr>> args;

    while (this->expect_token(Token::Type::RPAREN).is_invalid())
    {
        args.push_back(this->expr());

        this->expect_token(Token::Type::COMMA).adv_or_user_error("expected ',' after argument in argument list");
    }

    this->expect_token(Token::Type::RPAREN).adv_or_user_error("expected '(' after function call");

    return args;
}

std::unique_ptr<Expr> Parser::primary()
{
    if (this->expect_token(Token::Type::LPAREN).is_valid())
    {
        return this->grouping();
    }

    auto primary = this->expect_one_of({Token::Type::IDENTIFIER,
                                        Token::Type::INT_LITERAL,
                                        Token::Type::FLOAT_LITERAL,
                                        Token::Type::BOOL_LITERAL,
                                        Token::Type::STR_LITERAL})
                       .adv_or_user_error("expected identifier or literal value");

    return std::make_unique<Primary>(primary);
}

std::unique_ptr<Expr> Parser::grouping()
{
    this->expect_token(Token::Type::LPAREN).adv_or_bird_error("Expected '(' before grouping");

    auto expr = this->expr();

    this->expect_token(Token::Type::RPAREN).adv_or_user_error("expected ')' after grouping expression");

    return expr;
}

// FUNC: fn IDENTIFIER(?FN_PARAMS) ?FN_RETURN_TYPE BLOCK
std::unique_ptr<Stmt> Parser::func()
{
    this->expect_token(Token::Type::FN).adv_or_bird_error("Expected fn keyword");

    auto fn_identifier = this->expect_token(Token::Type::IDENTIFIER).adv_or_user_error("expected identifier after 'fn' keyword");

    this->expect_token(Token::Type::LPAREN).adv_or_user_error("expected '(' after identifier in function declaration");

    auto fn_params = this->fn_params();

    this->expect_token(Token::Type::RPAREN).adv_or_user_error("expected ')' after function parameter list");

    auto fn_return_type = this->fn_return_type();

    auto block = this->block();

    return std::make_unique<Func>(Func(fn_identifier, fn_return_type, fn_params, std::move(block)));
}

// FN_PARAMS: PARAM_DECL [, PARAM_DECL]*
std::vector<std::pair<Token, Token>> Parser::fn_params()
{
    if (this->expect_token(Token::Type::RPAREN).is_valid())
    {
        return {};
    }

    std::vector<std::pair<Token, Token>> params;

    while (true)
    {
        params.push_back(this->param_decl());

        if (this->expect_token(Token::Type::RPAREN).is_valid())
        {
            return params;
        }

        this->expect_token(Token::Type::COMMA).adv_or_user_error("expected ',' after function parameter");
    }
}

// PARAM_DECL: IDENTIFIER: TYPE_IDENTIFIER
std::pair<Token, Token> Parser::param_decl()
{
    auto identifier = this->expect_token(Token::Type::IDENTIFIER).adv_or_user_error("expected identifier in function parameter list");

    this->expect_token(Token::Type::COLON).adv_or_user_error("expected ':' after identifier in parameter declaration");

    auto type_identifier = this->expect_token(Token::Type::TYPE_IDENTIFIER).adv_or_user_error("expected type identifier after ':' in parameter declaration");

    return {identifier, type_identifier};
}

// FN_RETURN_TYPE: -> TYPE_IDENTIFIER
std::optional<Token> Parser::fn_return_type()
{
    if (this->expect_token(Token::Type::ARROW).is_invalid())
    {
        return std::nullopt;
    }

    this->advance();

    auto return_type = this->expect_token(Token::Type::TYPE_IDENTIFIER).adv_or_user_error("expected return type after '->' in function declaration");

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

Parser::ExpectedToken Parser::expect_token(Token::Type type)
{
    if (this->is_at_end())
    {
        this->user_error_tracker->unexpected_end_of_token_stream();
    }

    auto token = this->peek();
    if (token.token_type == type)
    {
        return ExpectedToken(token, *this);
    }

    return ExpectedToken(*this);
}

Parser::ExpectedToken Parser::expect_one_of(std::initializer_list<Token::Type> types)
{
    if (this->is_at_end())
    {
        this->user_error_tracker->unexpected_end_of_token_stream();
    }

    auto token = this->peek();
    for (auto type : types)
    {
        if (token.token_type == type)
        {
            return ExpectedToken(token, *this);
        }
    }

    return ExpectedToken(*this);
}

Token Parser::ExpectedToken::adv_or_user_error(std::string context)
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

Token Parser::ExpectedToken::adv_or_bird_error(std::string message)
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

bool Parser::ExpectedToken::is_valid()
{
    return (bool)token;
}

bool Parser::ExpectedToken::is_invalid()
{
    return !(bool)token;
}