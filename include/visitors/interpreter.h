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

#include "../sym_table.h"
#include "../exceptions/bird_exception.h"


#define HANDLE_GENERAL_BINARY_OPERATOR(left, right, data_type, bird_type, op) \
    if (std::holds_alternative<data_type>(left.data) && std::holds_alternative<data_type>(right.data)) \
    { \
        this->stack.push_back(Value(DataType::bird_type, variant(std::get<data_type>(left.data) op std::get<data_type>(right.data)))); \
        break; \
    }

#define HANDLE_NUMERIC_BINARY_OPERATOR(left, right, op) \
    if ((std::holds_alternative<int>(left.data) || std::holds_alternative<float>(left.data)) && (std::holds_alternative<int>(right.data) || std::holds_alternative<float>(right.data))) \
    { \
        float left_float = std::holds_alternative<int>(left.data) ? std::get<int>(left.data) : std::get<float>(left.data); \
        float right_float = std::holds_alternative<int>(right.data) ? std::get<int>(right.data) : std::get<float>(right.data); \
        \
        this->stack.push_back(Value(DataType::FLOAT, variant(left_float op right_float))); \
        break; \
    }

#define HANDLE_UNKNOWN_BINARY_OPERATOR(op) \
    throw BirdException("The '"#op"' binary operator could not be used to interpret these values.");


#define HANDLE_GENERAL_ASSIGN(data_type) \
    if (std::holds_alternative<data_type>(previous_value.data) && (std::holds_alternative<data_type>(value.data))) \
    { \
        this->environment->insert(assign_stmt->identifier.lexeme, value); \
        break; \
    }

#define HANDLE_GENERAL_COMPASSIGN_OPERATOR(data_type, bird_type, op) \
    if (std::holds_alternative<data_type>(previous_value.data) && (std::holds_alternative<data_type>(value.data))) \
    { \
        auto assign_value = Value(DataType::bird_type, variant(std::get<data_type>(previous_value.data) op std::get<data_type>(value.data))); \
        \
        this->environment->insert(assign_stmt->identifier.lexeme, assign_value); \
        break; \
    }

#define HANDLE_INT_COMPASSIGN_OPERATOR(op) \
    if (std::holds_alternative<int>(previous_value.data) && (std::holds_alternative<int>(value.data) ||std::holds_alternative<float>(value.data))) \
    { \
        auto value_data = std::holds_alternative<int>(value.data) ? std::get<int>(value.data) : static_cast<int>(std::get<float>(value.data)); \
        auto assign_value = Value(DataType::INT, variant(std::get<int>(previous_value.data) op value_data)); \
        \
        this->environment->insert(assign_stmt->identifier.lexeme, assign_value); \
        break; \
    }

#define HANDLE_FLOAT_COMPASSIGN_OPERATOR(op) \
    if (std::holds_alternative<float>(previous_value.data) && (std::holds_alternative<int>(value.data) ||std::holds_alternative<float>(value.data))) \
    { \
        auto value_data = std::holds_alternative<int>(value.data) ? std::get<int>(value.data) : std::get<float>(value.data); \
        auto assign_value = Value(DataType::FLOAT, variant(std::get<float>(previous_value.data) op value_data)); \
        \
        this->environment->insert(assign_stmt->identifier.lexeme, assign_value); \
        break; \
    }

#define HANDLE_UNKNOWN_COMPASSIGN_OPERATOR(op) \
    throw BirdException("The '"#op"'= assignment operator could not be used to interpret these values.");


enum DataType
{
    INT,
    FLOAT,
    STR,
    BOOL,
};

using variant = std::variant<int, float, std::string, bool>;

class Value
{
public:
    DataType data_type;
    std::variant<int, float, std::string, bool> data;
    bool is_mutable;

    Value(DataType data_type, variant data, bool is_mutable = false)
    {
        this->is_mutable = is_mutable;
        this->data = data;
        this->data_type = data_type;
    }

    Value() {};
};

/*
 * Visitor that interprets and evaluates the AST
 */
class Interpreter : public Visitor
{
    std::unique_ptr<SymbolTable<Value>> environment;
    std::vector<Value> stack;

public:
    Interpreter()
    {
        this->environment = std::make_unique<SymbolTable<Value>>(SymbolTable<Value>());
    }

    void evaluate(std::vector<std::unique_ptr<Stmt>> *stmts)
    {
        for (auto &stmt : *stmts)
        {
            if (auto decl_stmt = dynamic_cast<DeclStmt *>(stmt.get()))
            {
                decl_stmt->accept(this);
            }

            if (auto const_stmt = dynamic_cast<ConstStmt *>(stmt.get()))
            {
                const_stmt->accept(this);
            }

            if (auto assign_stmt = dynamic_cast<AssignStmt *>(stmt.get()))
            {
                assign_stmt->accept(this);
            }

            if (auto print_stmt = dynamic_cast<PrintStmt *>(stmt.get()))
            {
                print_stmt->accept(this);
            }

            if (auto block = dynamic_cast<Block *>(stmt.get()))
            {
                block->accept(this);
            }

            if (auto expr_stmt = dynamic_cast<ExprStmt *>(stmt.get()))
            {
                expr_stmt->accept(this);
            }

            if (auto while_stmt = dynamic_cast<WhileStmt *>(stmt.get()))
            {
                while_stmt->accept(this);
            }

            if (auto if_stmt = dynamic_cast<IfStmt *>(stmt.get()))
            {
                if_stmt->accept(this);
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

        auto result = this->stack.back();
        this->stack.pop_back();
        // This does not correctly update the is_mutable field and will be ignored until a fix is made.
        // result.is_mutable = true;

        if (decl_stmt->type_identifier.has_value())
        {
            std::string type_lexeme = decl_stmt->type_identifier.value().lexeme;

            // TODO: pass the UserErrorTracker into the interpreter so we can handle runtime errors
            if (type_lexeme == "int" && !std::holds_alternative<int>(result.data))
            {
                throw BirdException("mismatching type in assignment, expected int");
            }
            else if (type_lexeme == "float" && !std::holds_alternative<float>(result.data))
            {
                throw BirdException("mismatching type in assignment, expected float");
            }
            else if (type_lexeme == "str" && !std::holds_alternative<std::string>(result.data))
            {
                throw BirdException("mismatching type in assignment, expected str");
            }
            else if (type_lexeme == "bool" && !std::holds_alternative<bool>(result.data))
            {
                throw BirdException("mismatching type in assignment, expected bool");
            }
        }

        this->environment->insert(decl_stmt->identifier.lexeme, result);
    }

    void visit_assign_stmt(AssignStmt *assign_stmt)
    {
        if (!this->environment->contains(assign_stmt->identifier.lexeme))
        {
            throw BirdException("Identifier '" + assign_stmt->identifier.lexeme + "' is not initialized.");
        }

        assign_stmt->value->accept(this);
        auto value = this->stack.back();
        this->stack.pop_back();

        // Will ignore is_mutable field until a fix is made.
        // if (!value.is_mutable)
        // {
        //     throw BirdException("Identifier '" + assign_stmt->identifier.lexeme + "' is not mutable.");
        // }

        auto previous_value = this->environment->get(assign_stmt->identifier.lexeme);
        
        switch (assign_stmt->assign_operator.token_type)
        {
        case Token::Type::EQUAL:
        {
            if (std::holds_alternative<int>(previous_value.data) && (std::holds_alternative<int>(value.data) ||std::holds_alternative<float>(value.data)))
            {
                auto value_data = std::holds_alternative<int>(value.data) ? std::get<int>(value.data) : static_cast<int>(std::get<float>(value.data));
                auto assign_value = Value(DataType::INT, variant(value_data));

                this->environment->insert(assign_stmt->identifier.lexeme, assign_value);
                break;
            }
            if (std::holds_alternative<float>(previous_value.data) && (std::holds_alternative<int>(value.data) ||std::holds_alternative<float>(value.data)))
            {
                float value_data = std::holds_alternative<int>(value.data) ? std::get<int>(value.data) : std::get<float>(value.data);
                auto assign_value = Value(DataType::FLOAT, variant(value_data));

                this->environment->insert(assign_stmt->identifier.lexeme, assign_value);
                break;
            }
            HANDLE_GENERAL_ASSIGN(bool);
            HANDLE_GENERAL_ASSIGN(std::string);
            throw BirdException("The assigment value type does not match the identifer type.");
        }
        case Token::Type::PLUS_EQUAL:
        {
            HANDLE_INT_COMPASSIGN_OPERATOR(+);
            HANDLE_FLOAT_COMPASSIGN_OPERATOR(+);
            HANDLE_GENERAL_COMPASSIGN_OPERATOR(std::string, STR, +);
            HANDLE_UNKNOWN_COMPASSIGN_OPERATOR(+);
        }
        case Token::Type::MINUS_EQUAL:
        {
            HANDLE_INT_COMPASSIGN_OPERATOR(-);
            HANDLE_FLOAT_COMPASSIGN_OPERATOR(-);
            HANDLE_UNKNOWN_COMPASSIGN_OPERATOR(-);
        }
        case Token::Type::STAR_EQUAL:
        {
            HANDLE_INT_COMPASSIGN_OPERATOR(*);
            HANDLE_FLOAT_COMPASSIGN_OPERATOR(*);
            HANDLE_UNKNOWN_COMPASSIGN_OPERATOR(*);
        }
        case Token::Type::SLASH_EQUAL:
        {
            HANDLE_INT_COMPASSIGN_OPERATOR(/);
            HANDLE_FLOAT_COMPASSIGN_OPERATOR(/);
            HANDLE_UNKNOWN_COMPASSIGN_OPERATOR(/);
        }
        case Token::Type::PERCENT_EQUAL:
        {
            HANDLE_INT_COMPASSIGN_OPERATOR(%);
            HANDLE_UNKNOWN_COMPASSIGN_OPERATOR(%);
        }
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
            auto result = this->stack[this->stack.size() - 1];
            this->stack.pop_back();

            if (std::holds_alternative<int>(result.data))
            {
                std::cout << std::get<int>(result.data) << std::endl;
            }
            else if (std::holds_alternative<float>(result.data))
            {
                std::cout << std::get<float>(result.data) << std::endl;
            }
            else if (std::holds_alternative<std::string>(result.data))
            {
                std::cout << std::get<std::string>(result.data) << std::endl;
            }
            else if (std::holds_alternative<bool>(result.data))
            {
                std::cout << std::get<bool>(result.data) << std::endl;
            }
        }
        std::cout << std::endl;
    }

    void visit_const_stmt(ConstStmt *const_stmt)
    {
        const_stmt->value->accept(this);

        auto result = this->stack.back();
        this->stack.pop_back();

        if (const_stmt->type_identifier.has_value())
        {
            std::string type_lexeme = const_stmt->type_identifier.value().lexeme;

            // TODO: pass the UserErrorTracker into the interpreter so we can handle runtime errors
            if (type_lexeme == "int" && !std::holds_alternative<int>(result.data))
            {
                throw BirdException("mismatching type in assignment, expected int");
            }
            else if (type_lexeme == "float" && !std::holds_alternative<float>(result.data))
            {
                throw BirdException("mismatching type in assignment, expected float");
            }
            else if (type_lexeme == "str" && !std::holds_alternative<std::string>(result.data))
            {
                throw BirdException("mismatching type in assignment, expected str");
            }
            else if (type_lexeme == "bool" && !std::holds_alternative<bool>(result.data))
            {
                throw BirdException("mismatching type in assignment, expected bool");
            }
        }

        this->environment->insert(const_stmt->identifier.lexeme, result);
    }

    void visit_while_stmt(WhileStmt *while_stmt)
    {
        while_stmt->condition->accept(this);
        auto condition_result = this->stack.back();
        this->stack.pop_back();

        if (!std::holds_alternative<bool>(condition_result.data))
        {
            throw BirdException("expected bool in while statement condition");
        }

        while (std::get<bool>(condition_result.data))
        {
            while_stmt->stmt->accept(this);

            while_stmt->condition->accept(this);
            condition_result = this->stack.back();
            this->stack.pop_back();
        }
    }

    void visit_binary(Binary *binary)
    {
        binary->left->accept(this);
        binary->right->accept(this);

        auto right = this->stack.back();
        this->stack.pop_back();

        auto left = this->stack.back();
        this->stack.pop_back();

        switch (binary->op.token_type)
        {
        case Token::Type::PLUS:
        {
            HANDLE_GENERAL_BINARY_OPERATOR(left, right, int, INT, +);
            HANDLE_NUMERIC_BINARY_OPERATOR(left, right, +);
            HANDLE_GENERAL_BINARY_OPERATOR(left, right, std::string, STR, +);
            HANDLE_UNKNOWN_BINARY_OPERATOR(+);
        }
        case Token::Type::MINUS:
        {
            HANDLE_GENERAL_BINARY_OPERATOR(left, right, int, INT, -);
            HANDLE_NUMERIC_BINARY_OPERATOR(left, right, -);
            HANDLE_UNKNOWN_BINARY_OPERATOR(-);
        }
        case Token::Type::SLASH:
        {
            HANDLE_GENERAL_BINARY_OPERATOR(left, right, int, INT, /);
            HANDLE_NUMERIC_BINARY_OPERATOR(left, right, /);
            HANDLE_UNKNOWN_BINARY_OPERATOR(/);
        }
        case Token::Type::STAR:
        {
            HANDLE_GENERAL_BINARY_OPERATOR(left, right, int, INT, *);
            HANDLE_NUMERIC_BINARY_OPERATOR(left, right, *);
            HANDLE_UNKNOWN_BINARY_OPERATOR(*);
        }
        case Token::Type::GREATER:
        {
            HANDLE_GENERAL_BINARY_OPERATOR(left, right, int, INT, >);
            HANDLE_NUMERIC_BINARY_OPERATOR(left, right, >);
            HANDLE_UNKNOWN_BINARY_OPERATOR(>);
        }
        case Token::Type::GREATER_EQUAL:
        {
            HANDLE_GENERAL_BINARY_OPERATOR(left, right, int, INT, >=);
            HANDLE_NUMERIC_BINARY_OPERATOR(left, right, >=);
            HANDLE_UNKNOWN_BINARY_OPERATOR(>=);
        }
        case Token::Type::LESS:
        {
            HANDLE_GENERAL_BINARY_OPERATOR(left, right, int, INT, <);
            HANDLE_NUMERIC_BINARY_OPERATOR(left, right, <);
            HANDLE_UNKNOWN_BINARY_OPERATOR(<);
        }
        case Token::Type::LESS_EQUAL:
        {
            HANDLE_GENERAL_BINARY_OPERATOR(left, right, int, INT, <=);
            HANDLE_NUMERIC_BINARY_OPERATOR(left, right, <=);
            HANDLE_UNKNOWN_BINARY_OPERATOR(<=);
        }
        case Token::Type::BANG_EQUAL:
        {
            HANDLE_NUMERIC_BINARY_OPERATOR(left, right, !=);
            HANDLE_GENERAL_BINARY_OPERATOR(left, right, std::string, STR, !=);
            HANDLE_GENERAL_BINARY_OPERATOR(left, right, bool, BOOL, !=);
            HANDLE_UNKNOWN_BINARY_OPERATOR(!=);
        }
        case Token::Type::EQUAL_EQUAL:
        {
            HANDLE_NUMERIC_BINARY_OPERATOR(left, right, ==);
            HANDLE_GENERAL_BINARY_OPERATOR(left, right, std::string, STR, ==);
            HANDLE_GENERAL_BINARY_OPERATOR(left, right, bool, BOOL, ==);
            HANDLE_UNKNOWN_BINARY_OPERATOR(==);
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
        auto expr = this->stack[this->stack.size() - 1];
        this->stack.pop_back();

        if (std::holds_alternative<int>(expr.data))
        {
            this->stack.push_back(Value(DataType::INT, variant(-std::get<int>(expr.data))));
        }
        else if (std::holds_alternative<float>(expr.data))
        {
            this->stack.push_back(Value(DataType::FLOAT, variant(-std::get<float>(expr.data))));
        }
        else
        {
            throw BirdException("Unknown type used with unary value.");
        }
    }

    void visit_primary(Primary *primary)
    {
        switch (primary->value.token_type)
        {
        case Token::Type::FLOAT_LITERAL:
        {
            this->stack.push_back(Value(DataType::FLOAT,
                                        variant(std::stof(primary->value.lexeme))));
            break;
        }
        case Token::Type::BOOL_LITERAL:
        {
            this->stack.push_back(Value(DataType::BOOL,
                                        variant(primary->value.lexeme == "true" ? true : false)));
            break;
        }
        case Token::Type::STR_LITERAL:
        {
            this->stack.push_back(Value(DataType::STR,
                                        variant(primary->value.lexeme)));
            break;
        }
        case Token::Type::INT_LITERAL:
        {
            this->stack.push_back(Value(DataType::INT,
                                        variant(std::stoi(primary->value.lexeme))));
            break;
        }
        case Token::Type::IDENTIFIER:
        {
            auto value = this->environment->get(primary->value.lexeme);
            this->stack.push_back(value);
            break;
        }
        default:
        {
            throw BirdException("undefined primary value");
        }
        }
    }

    void visit_ternary(Ternary *ternary)
    {
        ternary->condition->accept(this);

        auto result = this->stack.back();

        this->stack.pop_back();

        if (!std::holds_alternative<bool>(result.data))
        {
            throw BirdException("expected bool result for ternary condition");
        }

        if (std::get<bool>(result.data))
        {
            ternary->true_expr->accept(this);
        }
        else
        {
            ternary->false_expr->accept(this);
        }
    }

    void visit_func(Func *func)
    {
        throw BirdException("implement func visit");
    }

    void visit_if_stmt(IfStmt *if_stmt)
    {
        if_stmt->condition->accept(this);

        auto result = this->stack.back();

        this->stack.pop_back();

        if (!std::holds_alternative<bool>(result.data))
        {
            throw BirdException("expected bool result for if-statement condition");
        }

        if (std::get<bool>(result.data))
        {
            if_stmt->then_branch->accept(this);
        }
        else
        {
            if (if_stmt->else_branch.has_value())
            {
                if_stmt->else_branch.value()->accept(this);
            }
        }
    }
};