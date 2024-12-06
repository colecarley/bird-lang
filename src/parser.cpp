// including in the .cpp due to a ciruclar dependency
#include "parser.h"
#include "ast_node/index.h"

#include "exceptions/bird_exception.h"
#include "exceptions/user_exception.h"
#include "exceptions/user_error_tracker.h"

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
    case Token::Type::FOR:
        return this->for_stmt();
    case Token::Type::RETURN:
        return this->return_stmt();
    case Token::Type::BREAK:
        return this->break_stmt();
    case Token::Type::CONTINUE:
        return this->continue_stmt();
    case Token::Type::TYPE:
        return this->type_stmt();
    default:
        break;
    }

    return this->expr_stmt();
}

std::unique_ptr<Stmt> Parser::return_stmt()
{
    if (this->peek().token_type != Token::Type::RETURN)
    {
        throw BirdException("Expected 'return' at the beginning of return stmt");
    }

    Token return_token = this->advance();

    if (this->peek().token_type == Token::Type::SEMICOLON)
    {
        this->advance();
        return std::make_unique<ReturnStmt>(ReturnStmt());
    }

    auto expr = this->expr();

    this->consume(Token::Type::SEMICOLON, ";", "after const statement", this->peek());

    return std::make_unique<ReturnStmt>(ReturnStmt(return_token, std::move(expr)));
}

std::unique_ptr<Stmt> Parser::const_decl()
{
    if (this->advance().token_type != Token::Type::CONST)
    {
        throw BirdException("Expected 'const' at the beginning of const decl");
    }

    Token identifier;
    if (this->peek().token_type == Token::Type::IDENTIFIER)
    {
        identifier = this->advance();
    }
    else
    {
        this->user_error_tracker->expected("identifier", "after const in declaration", this->peek());
        this->synchronize();
        throw UserException();
    }

    std::optional<Token> type_token = std::nullopt;
    bool type_is_literal = false;
    if (this->peek().token_type == Token::Type::COLON)
    {
        this->advance();
        type_is_literal = this->peek().token_type == Token::Type::TYPE_LITERAL ? true : false;

        if (this->peek().token_type == Token::Type::TYPE_LITERAL || this->peek().token_type == Token::Type::IDENTIFIER)
        {
            type_token = std::make_optional<Token>(this->advance());
        }
        else
        {
            this->user_error_tracker->expected("type literal or type identifer", "after : in assignment", this->peek());
            this->synchronize();
            throw UserException();
        }
    }

    this->consume(Token::Type::EQUAL, "=", "after type identifier", this->peek());

    auto expr = this->expr();

    this->consume(Token::Type::SEMICOLON, ";", "after const statement", this->peek());

    return std::make_unique<ConstStmt>(
        ConstStmt(
            identifier, type_token, type_is_literal, std::move(expr)));
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

    this->consume(Token::Type::RBRACE, "}", "at the end of block", this->peek_previous());

    return std::make_unique<Block>(Block(std::move(stmts)));
}

std::unique_ptr<Stmt> Parser::if_stmt()
{
    if (this->peek().token_type != Token::Type::IF)
    {
        throw BirdException("Expected 'if' at the beginning of if statement");
    }

    auto if_token = this->advance();

    auto condition = this->expr();

    auto statement = this->stmt();

    if (this->peek().token_type == Token::Type::ELSE)
    {
        this->advance();
        return std::make_unique<IfStmt>(
            if_token,
            std::move(condition),
            std::move(statement),
            std::make_optional<std::unique_ptr<Stmt>>(this->stmt()));
    }

    return std::make_unique<IfStmt>(
        if_token,
        std::move(condition),
        std::move(statement),
        std::nullopt);
}

std::unique_ptr<Stmt> Parser::expr_stmt()
{
    auto result = std::make_unique<ExprStmt>(
        ExprStmt(this->expr()));

    this->consume(Token::Type::SEMICOLON, ";", "at the end of expression", this->peek_previous());

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

    this->consume(Token::Type::SEMICOLON, ";", "after 'print'", this->peek_previous());

    auto result = std::make_unique<PrintStmt>(PrintStmt(std::move(values)));
    return result;
}

std::unique_ptr<Stmt> Parser::while_stmt()
{
    if (this->peek().token_type != Token::Type::WHILE)
    {
        throw BirdException("Expected 'while' at the beginning of while statement");
    }

    auto while_token = this->advance();

    auto condition = this->expr();

    auto stmt = this->stmt();

    return std::make_unique<WhileStmt>(WhileStmt(while_token, std::move(condition), std::move(stmt)));
}

std::unique_ptr<Stmt> Parser::for_stmt()
{
    if (this->peek().token_type != Token::Type::FOR)
    {
        throw BirdException("expected 'for' at the beginning of for statement");
    }

    auto for_token = this->advance();

    if (this->peek().token_type == Token::Type::LPAREN)
    {
        this->advance();
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

    if (this->peek().token_type == Token::Type::RPAREN)
    {
        this->advance();
    }

    if (this->advance().token_type != Token::Type::DO)
    {
        throw BirdException("expected 'do' at the end of for statement clauses");
    }

    auto body = this->stmt();

    return std::make_unique<ForStmt>(
        for_token,
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

    Token identifier;
    if (this->peek().token_type == Token::Type::IDENTIFIER)
    {
        identifier = this->advance();
    }
    else
    {
        this->user_error_tracker->expected("identifier", "after var in declaration", this->peek());
        this->synchronize();
        throw UserException();
    }

    std::optional<Token> type_token = std::nullopt;
    bool type_is_literal = false;
    if (this->peek().token_type == Token::Type::COLON)
    {
        this->advance();
        type_is_literal = this->peek().token_type == Token::Type::TYPE_LITERAL ? true : false;

        if (this->peek().token_type == Token::Type::TYPE_LITERAL || this->peek().token_type == Token::Type::IDENTIFIER)
        {
            type_token = std::make_optional<Token>(this->advance());
        }
        else
        {
            this->user_error_tracker->expected("type literal or type identifer", "after : in assignment", this->peek());
            this->synchronize();
            throw UserException();
        }
    }

    this->consume(Token::Type::EQUAL, "=", "in assignment", this->peek_previous());

    auto result = std::make_unique<DeclStmt>(
        DeclStmt(
            identifier,
            type_token,
            type_is_literal,
            this->expr()));

    this->consume(Token::Type::SEMICOLON, ";", "at the end of expression", this->peek_previous());

    return result;
}

std::unique_ptr<Expr> Parser::assign_expr()
{
    auto left = this->ternary();
    // TODO: check if left is an identifier, return if not;

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
            throw BirdException("can not assign value to non-identifier");
        }
    }

    return left;
}

std::unique_ptr<Stmt> Parser::break_stmt()
{
    if (this->peek().token_type != Token::Type::BREAK)
    {
        throw BirdException("Expected 'break' at the beginning of break stmt");
    }

    Token break_token = this->advance();

    this->consume(Token::Type::SEMICOLON, ";", "at the end of expression", this->peek_previous());

    return std::make_unique<BreakStmt>(break_token);
}

std::unique_ptr<Stmt> Parser::continue_stmt()
{
    if (this->peek().token_type != Token::Type::CONTINUE)
    {
        throw BirdException("Expected 'continue' at the beginning of continue stmt");
    }

    Token continue_token = this->advance();

    this->consume(Token::Type::SEMICOLON, ";", "at the end of expression", this->peek_previous());

    return std::make_unique<ContinueStmt>(continue_token);
}

std::unique_ptr<Stmt> Parser::type_stmt()
{
    if (this->advance().token_type != Token::Type::TYPE)
    {
        throw BirdException("Expected 'type' at the beginning of type stmt");
    }

    Token type_identifier;
    if (this->peek().token_type == Token::Type::IDENTIFIER)
    {
        type_identifier = this->advance();
    }
    else
    {
        this->user_error_tracker->expected("identifier", "after type in type statement", this->peek());
        this->synchronize();
        throw UserException();
    }

    this->consume(Token::Type::EQUAL, "=", "in assignment", this->peek_previous());

    Token type_token;
    bool type_is_literal = this->peek().token_type == Token::Type::TYPE_LITERAL ? true : false;
    if (this->peek().token_type == Token::Type::TYPE_LITERAL || this->peek().token_type == Token::Type::IDENTIFIER)
    {
        type_token = this->advance();
    }
    else
    {
        this->user_error_tracker->expected("type literal or type identifer", "after = in type statement", this->peek());
        this->synchronize();
        throw UserException();
    }

    this->consume(Token::Type::SEMICOLON, ";", "at the end of expression", this->peek_previous());

    return std::make_unique<TypeStmt>(TypeStmt(type_identifier, type_token, type_is_literal));
}

std::unique_ptr<Expr> Parser::expr()
{
    return this->assign_expr();
}

std::unique_ptr<Expr> Parser::ternary()
{
    auto condition = this->equality();

    if (this->peek().token_type == Token::Type::QUESTION)
    {
        Token ternary_token = this->advance();

        auto true_expr = this->expr();

        this->consume(Token::Type::COLON, ":", "after true expression in ternary operator", this->peek_previous());

        auto false_expr = this->expr();

        return std::make_unique<Ternary>(
            std::move(condition),
            ternary_token,
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

std::vector<std::shared_ptr<Expr>> Parser::args()
{
    this->advance(); // advance past '('
    std::vector<std::shared_ptr<Expr>> args;

    while (this->peek().token_type != Token::Type::RPAREN)
    {
        args.push_back(this->expr());

        if (this->peek().token_type == Token::Type::COMMA)
            this->advance();
    }

    this->consume(Token::Type::RPAREN, ")", "after function call", this->peek());

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

    this->consume(Token::Type::RPAREN, ")", "after grouping", this->peek_previous());

    return expr;
}

std::unique_ptr<Stmt> Parser::func()
{
    if (this->advance().token_type != Token::Type::FN)
    {
        throw BirdException("expected fn keyword");
    }

    auto fn_identifier = this->peek();

    this->consume(Token::Type::IDENTIFIER, "identifier", "after keyword", this->peek_previous());
    this->consume(Token::Type::LPAREN, "(", "after identifier", this->peek_previous());

    auto fn_params = this->fn_params();

    this->consume(Token::Type::RPAREN, ")", "after function parameter list", this->peek_previous());

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
    auto identifier = this->peek();

    this->consume(Token::Type::IDENTIFIER, "identifier", "in function parameter list", this->peek_previous());

    this->consume(Token::Type::COLON, ":", "after identifier in parameter declaration", this->peek_previous());

    auto type_identifier = this->peek();

    if (type_identifier.token_type == Token::Type::IDENTIFIER)
    {
        this->consume(Token::Type::IDENTIFIER, "type", "after \':\' in parameter declaration", this->peek_previous());
        return {identifier, type_identifier};
    }
    this->consume(Token::Type::TYPE_LITERAL, "type", "after \':\' in parameter declaration", this->peek_previous());

    return {identifier, type_identifier};
}

std::optional<Token> Parser::fn_return_type()
{
    if (this->peek().token_type != Token::Type::ARROW)
    {
        return {};
    }

    this->advance();

    auto return_type = this->peek();

    if (return_type.token_type == Token::Type::IDENTIFIER)
    {
        this->consume(Token::Type::IDENTIFIER, "type", "after arrow", this->peek_previous());
        return return_type;
    }

    this->consume(Token::Type::TYPE_LITERAL, "type", "after arrow", this->peek_previous());

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

// Maybe 'token_type_string' could be replaced with a function that retrieves the string of a 'Token::Type'.
void Parser::consume(Token::Type expected_type, std::string token_type_string, std::string where, Token token)
{
    if (this->advance().token_type != expected_type)
    {
        user_error_tracker->expected(token_type_string, where, token);
        this->synchronize();
        throw UserException();
    }
}