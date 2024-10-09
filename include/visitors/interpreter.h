#pragma once

#include <memory>
#include <vector>
#include <variant>

#include "../ast_node/stmt/stmt.h"
#include "../ast_node/expr/expr.h"

#include "../ast_node/expr/binary.h"
#include "../ast_node/expr/unary.h"
#include "../ast_node/expr/primary.h"
#include "../ast_node/expr/ternary.h"

#include "../ast_node/stmt/decl_stmt.h"
#include "../ast_node/stmt/assign_stmt.h"
#include "../ast_node/stmt/expr_stmt.h"
#include "../ast_node/stmt/print_stmt.h"
#include "../ast_node/stmt/const_stmt.h"
#include "../ast_node/stmt/while_stmt.h"
#include "../ast_node/stmt/if_stmt.h"
#include "../ast_node/stmt/block.h"
#include "../ast_node/stmt/func.h"

#include "../sym_table.h"
#include "../exceptions/bird_exception.h"

#define HANDLE_GENERAL_BINARY_OPERATOR(left, right, data_type, op) \
    if (is_type<data_type>(left.data) &&                           \
        is_type<data_type>(right.data))                            \
    {                                                              \
        this->stack.push_back(Value(                               \
            variant(as_type<data_type>(left.data)                  \
                        op as_type<data_type>(right.data))));      \
        break;                                                     \
    }

#define HANDLE_NUMERIC_BINARY_OPERATOR(left, right, op)            \
    if (is_numeric(left) && is_numeric(right))                     \
    {                                                              \
        float left_float = is_type<int>(left.data)                 \
                               ? as_type<int>(left.data)           \
                               : as_type<float>(left.data);        \
        float right_float = is_type<int>(right.data)               \
                                ? as_type<int>(right.data)         \
                                : as_type<float>(right.data);      \
                                                                   \
        this->stack.push_back(Value(variant(left_float             \
                                                op right_float))); \
        break;                                                     \
    }

#define THROW_UNKNWOWN_BINARY_OPERATOR(op) \
    throw BirdException("The '" #op "' binary operator could not be used to interpret these values.");

#define THROW_UNKNWOWN_COMPASSIGN_OPERATOR(op) \
    throw BirdException("The '" #op "'= assignment operator could not be used to interpret these values.");

using variant = std::variant<int, float, std::string, bool>;

class Value
{
public:
    variant data;
    bool is_mutable;

    Value(variant data, bool is_mutable = false) : data(std::move(data)), is_mutable(is_mutable) {}

    Value() {};
};

template <typename T>
static inline bool is_type(Value value)
{
    return std::holds_alternative<T>(value.data);
}

static inline bool is_numeric(Value value)
{
    return is_type<int>(value) || is_type<float>(value);
}

template <typename T>
static inline bool is_matching_type(Value left, Value right)
{
    return is_type<T>(left) && is_type<T>(right);
}

template <typename T>
static inline T as_type(Value value)
{
    return std::get<T>(value.data);
}

template <typename T, typename U>
static inline T to_type(Value value)
{
    return is_type<T>(value) ? as_type<T>(value) : static_cast<T>(as_type<U>(value));
}

class Callable
{
    std::vector<std::pair<Token, Token>> param_list;
    std::unique_ptr<Stmt> block;
    std::optional<Token> return_type;

public:
    Callable(
        std::vector<std::pair<Token, Token>> param_list,
        std::unique_ptr<Stmt> block,
        std::optional<Token> return_type)
        : param_list(param_list),
          block(std::move(block)),
          return_type(return_type) {}
    Callable() = default;
};

/*
 * Visitor that interprets and evaluates the AST
 */
class Interpreter : public Visitor
{
    std::unique_ptr<SymbolTable<Value>> environment;
    std::unique_ptr<SymbolTable<Callable>> call_table;
    std::vector<Value> stack;

public:
    Interpreter()
    {
        this->environment = std::make_unique<SymbolTable<Value>>(SymbolTable<Value>());
        this->call_table = std::make_unique<SymbolTable<Callable>>(SymbolTable<Callable>());
    }

    void evaluate(std::vector<std::unique_ptr<Stmt>> *stmts)
    {
        for (auto &stmt : *stmts)
        {
            if (auto decl_stmt = dynamic_cast<DeclStmt *>(stmt.get()))
            {
                decl_stmt->accept(this);
                continue;
            }

            if (auto const_stmt = dynamic_cast<ConstStmt *>(stmt.get()))
            {
                const_stmt->accept(this);
                continue;
            }

            if (auto assign_stmt = dynamic_cast<AssignStmt *>(stmt.get()))
            {
                assign_stmt->accept(this);
                continue;
            }

            if (auto print_stmt = dynamic_cast<PrintStmt *>(stmt.get()))
            {
                print_stmt->accept(this);
                continue;
            }

            if (auto block = dynamic_cast<Block *>(stmt.get()))
            {
                block->accept(this);
                continue;
            }

            if (auto expr_stmt = dynamic_cast<ExprStmt *>(stmt.get()))
            {
                expr_stmt->accept(this);
                continue;
            }

            if (auto while_stmt = dynamic_cast<WhileStmt *>(stmt.get()))
            {
                while_stmt->accept(this);
                continue;
            }

            if (auto if_stmt = dynamic_cast<IfStmt *>(stmt.get()))
            {
                if_stmt->accept(this);
                continue;
            }

            if (auto func = dynamic_cast<Func *>(stmt.get()))
            {
                func->accept(this);
                continue;
            }
        }

        this->stack.clear();
    }

    void visit_block(Block *block)
    {
        auto new_environment = std::make_unique<SymbolTable<Value>>(SymbolTable<Value>());
        new_environment->set_enclosing(std::move(this->environment));
        this->environment = std::move(new_environment);

        for (auto &stmt : block->stmts)
        {
            stmt->accept(this);
        }

        this->environment = this->environment->get_enclosing();
    }

    void visit_decl_stmt(DeclStmt *decl_stmt)
    {
        decl_stmt->value->accept(this);

        auto result = std::move(this->stack.back());
        this->stack.pop_back();
        result.is_mutable = true;

        if (decl_stmt->type_identifier.has_value())
        {
            std::string type_lexeme = decl_stmt->type_identifier.value().lexeme;

            // TODO: pass the UserErrorTracker into the interpreter so we can handle runtime errors
            if (type_lexeme == "int" && !is_type<int>(result.data))
                throw BirdException("mismatching type in assignment, expected int");
            else if (type_lexeme == "float" && !is_type<float>(result.data))
                throw BirdException("mismatching type in assignment, expected float");
            else if (type_lexeme == "str" && !is_type<std::string>(result.data))
                throw BirdException("mismatching type in assignment, expected str");
            else if (type_lexeme == "bool" && !is_type<bool>(result.data))
                throw BirdException("mismatching type in assignment, expected bool");
        }

        this->environment->insert(decl_stmt->identifier.lexeme, std::move(result));
    }

    void visit_assign_stmt(AssignStmt *assign_stmt)
    {
        if (!this->environment->contains(assign_stmt->identifier.lexeme))
            throw BirdException("Identifier '" + assign_stmt->identifier.lexeme + "' is not initialized.");

        auto previous_value = this->environment->get(assign_stmt->identifier.lexeme);
        if (!previous_value.is_mutable)
            throw BirdException("Identifier '" + assign_stmt->identifier.lexeme + "' is not mutable.");

        assign_stmt->value->accept(this);
        auto value = std::move(this->stack.back());
        this->stack.pop_back();
        // value.is_mutable = true;

        switch (assign_stmt->assign_operator.token_type)
        {
        case Token::Type::EQUAL:
        {
            if (is_matching_type<bool>(previous_value, value) ||
                is_matching_type<std::string>(previous_value, value))
                previous_value.data = value.data;

            else if (is_type<int>(previous_value) && is_numeric(value))
                previous_value.data = to_type<int, float>(value);

            else if (is_type<float>(previous_value) && is_numeric(value))
                previous_value.data = to_type<float, int>(value);

            else
                throw BirdException("The assigment value type does not match the identifer type.");

            this->environment->insert(assign_stmt->identifier.lexeme, previous_value);
            break;
        }
        case Token::Type::PLUS_EQUAL:
        {
            if (is_matching_type<std::string>(previous_value, value))
                previous_value.data = as_type<std::string>(previous_value) + as_type<std::string>(value);

            else if (is_type<int>(previous_value) && is_numeric(value))
                previous_value.data = as_type<int>(previous_value) + to_type<int, float>(value);

            else if (is_type<float>(previous_value) && is_numeric(value))
                previous_value.data = as_type<float>(previous_value) + to_type<float, int>(value);

            else
                THROW_UNKNWOWN_COMPASSIGN_OPERATOR(+);

            this->environment->insert(assign_stmt->identifier.lexeme, previous_value);
            break;
        }
        case Token::Type::MINUS_EQUAL:
        {
            if (is_type<int>(previous_value) && is_numeric(value))
                previous_value.data = as_type<int>(previous_value) - to_type<int, float>(value);

            else if (is_type<float>(previous_value) && is_numeric(value))
                previous_value.data = as_type<float>(previous_value) - to_type<float, int>(value);

            else
                THROW_UNKNWOWN_COMPASSIGN_OPERATOR(-);

            this->environment->insert(assign_stmt->identifier.lexeme, previous_value);
            break;
        }
        case Token::Type::STAR_EQUAL:
        {
            if (is_type<int>(previous_value) && is_numeric(value))
                previous_value.data = as_type<int>(previous_value) * to_type<int, float>(value);

            else if (is_type<float>(previous_value) && is_numeric(value))
                previous_value.data = as_type<float>(previous_value) * to_type<float, int>(value);

            else
                THROW_UNKNWOWN_COMPASSIGN_OPERATOR(*);

            this->environment->insert(assign_stmt->identifier.lexeme, previous_value);
            break;
        }
        case Token::Type::SLASH_EQUAL:
        {
            if (is_type<int>(previous_value) && is_numeric(value))
                previous_value.data = as_type<int>(previous_value) / to_type<int, float>(value);

            else if (is_type<float>(previous_value) && is_numeric(value))
                previous_value.data = as_type<float>(previous_value) / to_type<float, int>(value);

            else
                THROW_UNKNWOWN_COMPASSIGN_OPERATOR(/);

            this->environment->insert(assign_stmt->identifier.lexeme, previous_value);
            break;
        }
        case Token::Type::PERCENT_EQUAL:
        {
            if (is_type<int>(previous_value) && is_numeric(value))
                previous_value.data = as_type<int>(previous_value) % to_type<int, float>(value);

            else
                THROW_UNKNWOWN_COMPASSIGN_OPERATOR(%);

            this->environment->insert(assign_stmt->identifier.lexeme, previous_value);
            break;
        }
        default:
            throw BirdException("Unidentified assignment operator " + assign_stmt->assign_operator.lexeme);
        }
    }

    void visit_expr_stmt(ExprStmt *expr_stmt)
    {
        expr_stmt->expr->accept(this);
    }

    void visit_print_stmt(PrintStmt *print_stmt)
    {
        for (auto &arg : print_stmt->args)
        {
            arg->accept(this);
            auto result = std::move(this->stack.back());
            this->stack.pop_back();

            if (is_type<int>(result.data))
                std::cout << as_type<int>(result.data);

            else if (is_type<float>(result.data))
                std::cout << as_type<float>(result.data);

            else if (is_type<std::string>(result.data))
                std::cout << as_type<std::string>(result.data);

            else if (is_type<bool>(result.data))
                std::cout << as_type<bool>(result.data);
        }
        std::cout << std::endl;
    }

    void visit_const_stmt(ConstStmt *const_stmt)
    {
        const_stmt->value->accept(this);

        auto result = std::move(this->stack.back());
        this->stack.pop_back();

        if (const_stmt->type_identifier.has_value())
        {
            std::string type_lexeme = const_stmt->type_identifier.value().lexeme;

            // TODO: pass the UserErrorTracker into the interpreter so we can handle runtime errors
            if (type_lexeme == "int" && !is_type<int>(result.data))
                throw BirdException("mismatching type in assignment, expected int");

            else if (type_lexeme == "float" && !is_type<float>(result.data))
                throw BirdException("mismatching type in assignment, expected float");

            else if (type_lexeme == "str" && !is_type<std::string>(result.data))
                throw BirdException("mismatching type in assignment, expected str");

            else if (type_lexeme == "bool" && !is_type<bool>(result.data))
                throw BirdException("mismatching type in assignment, expected bool");
        }

        this->environment->insert(const_stmt->identifier.lexeme, std::move(result));
    }

    void visit_while_stmt(WhileStmt *while_stmt)
    {
        while_stmt->condition->accept(this);
        auto condition_result = std::move(this->stack.back());
        this->stack.pop_back();

        if (!is_type<bool>(condition_result.data))
            throw BirdException("expected bool in while statement condition");

        while (as_type<bool>(condition_result.data))
        {
            while_stmt->stmt->accept(this);

            while_stmt->condition->accept(this);
            condition_result = std::move(this->stack.back());
            this->stack.pop_back();
        }
    }

    void visit_binary(Binary *binary)
    {
        binary->left->accept(this);
        binary->right->accept(this);

        auto right = std::move(this->stack.back());
        this->stack.pop_back();

        auto left = std::move(this->stack.back());
        this->stack.pop_back();

        switch (binary->op.token_type)
        {
        case Token::Type::PLUS:
        {
            HANDLE_GENERAL_BINARY_OPERATOR(left, right, int, +);
            HANDLE_NUMERIC_BINARY_OPERATOR(left, right, +);
            HANDLE_GENERAL_BINARY_OPERATOR(left, right, std::string, +);
            THROW_UNKNWOWN_BINARY_OPERATOR(+);
        }
        case Token::Type::MINUS:
        {
            HANDLE_GENERAL_BINARY_OPERATOR(left, right, int, -);
            HANDLE_NUMERIC_BINARY_OPERATOR(left, right, -);
            THROW_UNKNWOWN_BINARY_OPERATOR(-);
        }
        case Token::Type::SLASH:
        {
            HANDLE_GENERAL_BINARY_OPERATOR(left, right, int, /);
            HANDLE_NUMERIC_BINARY_OPERATOR(left, right, /);
            THROW_UNKNWOWN_BINARY_OPERATOR(/);
        }
        case Token::Type::STAR:
        {
            HANDLE_GENERAL_BINARY_OPERATOR(left, right, int, *);
            HANDLE_NUMERIC_BINARY_OPERATOR(left, right, *);
            THROW_UNKNWOWN_BINARY_OPERATOR(*);
        }
        case Token::Type::GREATER:
        {
            HANDLE_GENERAL_BINARY_OPERATOR(left, right, int, >);
            HANDLE_NUMERIC_BINARY_OPERATOR(left, right, >);
            THROW_UNKNWOWN_BINARY_OPERATOR(>);
        }
        case Token::Type::GREATER_EQUAL:
        {
            HANDLE_GENERAL_BINARY_OPERATOR(left, right, int, >=);
            HANDLE_NUMERIC_BINARY_OPERATOR(left, right, >=);
            THROW_UNKNWOWN_BINARY_OPERATOR(>=);
        }
        case Token::Type::LESS:
        {
            HANDLE_GENERAL_BINARY_OPERATOR(left, right, int, <);
            HANDLE_NUMERIC_BINARY_OPERATOR(left, right, <);
            THROW_UNKNWOWN_BINARY_OPERATOR(<);
        }
        case Token::Type::LESS_EQUAL:
        {
            HANDLE_GENERAL_BINARY_OPERATOR(left, right, int, <=);
            HANDLE_NUMERIC_BINARY_OPERATOR(left, right, <=);
            THROW_UNKNWOWN_BINARY_OPERATOR(<=);
        }
        case Token::Type::BANG_EQUAL:
        {
            HANDLE_NUMERIC_BINARY_OPERATOR(left, right, !=);
            HANDLE_GENERAL_BINARY_OPERATOR(left, right, std::string, !=);
            HANDLE_GENERAL_BINARY_OPERATOR(left, right, bool, !=);
            THROW_UNKNWOWN_BINARY_OPERATOR(!=);
        }
        case Token::Type::EQUAL_EQUAL:
        {
            HANDLE_NUMERIC_BINARY_OPERATOR(left, right, ==);
            HANDLE_GENERAL_BINARY_OPERATOR(left, right, std::string, ==);
            HANDLE_GENERAL_BINARY_OPERATOR(left, right, bool, ==);
            THROW_UNKNWOWN_BINARY_OPERATOR(==);
        }
        default:
        {
            throw BirdException("Undefined binary operator.");
        }
        }
    }

    void visit_unary(Unary *unary)
    {
        unary->expr->accept(this);
        auto expr = std::move(this->stack.back());
        this->stack.pop_back();

        if (is_type<int>(expr))
            this->stack.push_back(Value(variant(-as_type<int>(expr.data))));
        else if (is_type<float>(expr))
            this->stack.push_back(Value(variant(-as_type<float>(expr.data))));
        else
            throw BirdException("Unknown type used with unary value.");
    }

    void visit_primary(Primary *primary)
    {
        switch (primary->value.token_type)
        {
        case Token::Type::FLOAT_LITERAL:
            this->stack.push_back(Value(
                variant(std::stof(primary->value.lexeme))));
            break;
        case Token::Type::BOOL_LITERAL:
            this->stack.push_back(Value(
                variant(primary->value.lexeme == "true" ? true : false)));
            break;
        case Token::Type::STR_LITERAL:
            this->stack.push_back(Value(
                variant(primary->value.lexeme)));
            break;
        case Token::Type::INT_LITERAL:
            this->stack.push_back(Value(
                variant(std::stoi(primary->value.lexeme))));
            break;
        case Token::Type::IDENTIFIER:
            this->stack.push_back(
                this->environment->get(primary->value.lexeme));
            break;
        default:
            throw BirdException("undefined primary value");
        }
    }

    void visit_ternary(Ternary *ternary)
    {
        ternary->condition->accept(this);

        auto result = std::move(this->stack.back());
        this->stack.pop_back();

        if (!is_type<bool>(result.data))
            throw BirdException("expected bool result for ternary condition");

        if (as_type<bool>(result.data))
            ternary->true_expr->accept(this);
        else
            ternary->false_expr->accept(this);
    }

    void visit_func(Func *func)
    {
        Callable callable = Callable(func->param_list, std::move(func->block), func->return_type);
        this->call_table.get()->insert(func->identifier.lexeme, std::move(callable));
    }

    void visit_if_stmt(IfStmt *if_stmt)
    {
        if_stmt->condition->accept(this);

        auto result = std::move(this->stack.back());
        this->stack.pop_back();

        if (!is_type<bool>(result.data))
            throw BirdException("expected bool result for if-statement condition");

        if (as_type<bool>(result.data))
            if_stmt->then_branch->accept(this);
        else if (if_stmt->else_branch.has_value())
            if_stmt->else_branch.value()->accept(this);
    }
};