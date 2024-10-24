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
#include "../ast_node/expr/call.h"

#include "../ast_node/stmt/decl_stmt.h"
#include "../ast_node/expr/assign_expr.h"
#include "../ast_node/stmt/expr_stmt.h"
#include "../ast_node/stmt/print_stmt.h"
#include "../ast_node/stmt/const_stmt.h"
#include "../ast_node/stmt/while_stmt.h"
#include "../ast_node/stmt/for_stmt.h"
#include "../ast_node/stmt/return_stmt.h"
#include "../ast_node/stmt/if_stmt.h"
#include "../ast_node/stmt/block.h"
#include "../ast_node/stmt/func.h"
#include "../ast_node/stmt/break_stmt.h"
#include "../ast_node/stmt/continue_stmt.h"

#include "../sym_table.h"
#include "../exceptions/bird_exception.h"
#include "../exceptions/return_exception.h"
#include "../exceptions/break_exception.h"
#include "../exceptions/continue_exception.h"
#include "../value.h"
#include "../callable.h"

#define HANDLE_GENERAL_BINARY_OPERATOR(left, right, data_type, op) \
    if (is_type<data_type>(left) &&                                \
        is_type<data_type>(right))                                 \
    {                                                              \
        this->stack.push(Value(                                    \
            variant(as_type<data_type>(left)                       \
                        op as_type<data_type>(right))));           \
        break;                                                     \
    }

#define HANDLE_NUMERIC_BINARY_OPERATOR(left, right, op)       \
    if (is_numeric(left) && is_numeric(right))                \
    {                                                         \
        float left_float = is_type<int>(left)                 \
                               ? as_type<int>(left)           \
                               : as_type<float>(left);        \
        float right_float = is_type<int>(right)               \
                                ? as_type<int>(right)         \
                                : as_type<float>(right);      \
                                                              \
        this->stack.push(Value(variant(left_float             \
                                           op right_float))); \
        break;                                                \
    }

#define THROW_UNKNWOWN_BINARY_OPERATOR(op) \
    throw BirdException("The '" #op "' binary operator could not be used to interpret these values.");

#define THROW_UNKNWOWN_COMPASSIGN_OPERATOR(op) \
    throw BirdException("The '" #op "'= assignment operator could not be used to interpret these values.");

/*
 * Visitor that interprets and evaluates the AST
 */
class Interpreter : public Visitor
{

public:
    std::shared_ptr<SymbolTable<Value>> environment;
    std::shared_ptr<SymbolTable<Callable>> call_table;
    std::stack<Value> stack;

    Interpreter()
    {
        this->environment = std::make_shared<SymbolTable<Value>>();
        this->call_table = std::make_shared<SymbolTable<Callable>>();
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

            if (auto assign_expr = dynamic_cast<AssignExpr *>(stmt.get()))
            {
                assign_expr->accept(this);
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

            if (auto for_stmt = dynamic_cast<ForStmt *>(stmt.get()))
            {
                for_stmt->accept(this);
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

            if (auto return_stmt = dynamic_cast<ReturnStmt *>(stmt.get()))
            {
                return_stmt->accept(this);
            }

            if (auto break_stmt = dynamic_cast<BreakStmt *>(stmt.get()))
            {
                break_stmt->accept(this);
                continue;
            }

            if (auto continue_stmt = dynamic_cast<ContinueStmt *>(stmt.get()))
            {
                continue_stmt->accept(this);
                continue;
            }
        }

        while (!this->stack.empty())
        {
            this->stack.pop();
        }
    }

    void visit_block(Block *block)
    {
        std::shared_ptr<SymbolTable<Value>> new_environment = std::make_shared<SymbolTable<Value>>();
        new_environment->set_enclosing(this->environment);
        this->environment = new_environment;

        for (auto &stmt : block->stmts)
        {
            stmt->accept(this);
        }

        this->environment = this->environment->get_enclosing();
    }

    void visit_decl_stmt(DeclStmt *decl_stmt)
    {
        std::shared_ptr<SymbolTable<Value>> current_env = this->environment;

        while (current_env)
        {
            if (this->environment->contains(decl_stmt->identifier.lexeme))
            {
                throw BirdException("Identifier '" + decl_stmt->identifier.lexeme + "' is already declared.");
            }

            current_env = current_env->get_enclosing();
        }

        decl_stmt->value->accept(this);

        auto result = std::move(this->stack.top());
        this->stack.pop();
        result.is_mutable = true;

        if (decl_stmt->type_identifier.has_value())
        {
            std::string type_lexeme = decl_stmt->type_identifier.value().lexeme;

            // TODO: pass the UserErrorTracker into the interpreter so we can handle runtime errors
            if (type_lexeme == "int")
            {
                if (!is_numeric(result))
                    throw BirdException("mismatching type in assignment, expected int");
                else
                    result.data = to_type<int, float>(result);
            }
            else if (type_lexeme == "float")
            {
                if (!is_numeric(result))
                    throw BirdException("mismatching type in assignment, expected float");
                else
                    result.data = to_type<float, int>(result);
            }
            else if (type_lexeme == "str" && !is_type<std::string>(result))
                throw BirdException("mismatching type in assignment, expected str");
            else if (type_lexeme == "bool" && !is_type<bool>(result))
                throw BirdException("mismatching type in assignment, expected bool");
        }

        this->environment->insert(decl_stmt->identifier.lexeme, std::move(result));
    }

    void visit_assign_expr(AssignExpr *assign_expr)
    {
        std::shared_ptr<SymbolTable<Value>> current_env = this->environment;

        while (current_env && !current_env->contains(assign_expr->identifier.lexeme))
        {
            current_env = current_env->get_enclosing();
        }

        if (!current_env)
        {
            throw BirdException("Identifier '" + assign_expr->identifier.lexeme + "' is not initialized.");
        }

        auto previous_value = current_env->get(assign_expr->identifier.lexeme);

        if (!previous_value.is_mutable)
        {
            throw BirdException("Identifier '" + assign_expr->identifier.lexeme + "' is not mutable.");
        }

        assign_expr->value->accept(this);
        auto value = std::move(this->stack.top());
        this->stack.pop();

        switch (assign_expr->assign_operator.token_type)
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

            break;
        }
        case Token::Type::SLASH_EQUAL:
        {
            if ((is_type<int>(value) && as_type<int>(value) == 0) || (is_type<float>(value) && as_type<float>(value) == 0))
            {
                throw BirdException("Divide by 0 error.");
            }

            if (is_type<int>(previous_value) && is_numeric(value))
                previous_value.data = as_type<int>(previous_value) / to_type<int, float>(value);

            else if (is_type<float>(previous_value) && is_numeric(value))
                previous_value.data = as_type<float>(previous_value) / to_type<float, int>(value);

            else
                THROW_UNKNWOWN_COMPASSIGN_OPERATOR(/);

            break;
        }
        case Token::Type::PERCENT_EQUAL:
        {
            if (is_type<int>(previous_value) && is_numeric(value))
                previous_value.data = as_type<int>(previous_value) % to_type<int, float>(value);

            else
                THROW_UNKNWOWN_COMPASSIGN_OPERATOR(%);

            break;
        }
        default:
            throw BirdException("Unidentified assignment operator " + assign_expr->assign_operator.lexeme);
        }

        current_env->insert(assign_expr->identifier.lexeme, previous_value);
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
            auto result = std::move(this->stack.top());
            this->stack.pop();

            if (is_type<int>(result))
                std::cout << as_type<int>(result);

            else if (is_type<float>(result))
                std::cout << as_type<float>(result);

            else if (is_type<std::string>(result))
            {
                std::cout << as_type<std::string>(result.data);
            }

            else if (is_type<bool>(result))
                std::cout << as_type<bool>(result);
        }
        std::cout << std::endl;
    }

    void visit_const_stmt(ConstStmt *const_stmt)
    {
        std::shared_ptr<SymbolTable<Value>> current_env = this->environment;

        while (current_env)
        {
            if (this->environment->contains(const_stmt->identifier.lexeme))
            {
                throw BirdException("Identifier '" + const_stmt->identifier.lexeme + "' is already declared.");
            }

            current_env = current_env->get_enclosing();
        }

        const_stmt->value->accept(this);

        auto result = std::move(this->stack.top());
        this->stack.pop();

        if (const_stmt->type_identifier.has_value())
        {
            std::string type_lexeme = const_stmt->type_identifier.value().lexeme;

            // TODO: pass the UserErrorTracker into the interpreter so we can handle runtime errors
            if (type_lexeme == "int")
            {
                is_numeric(result) ? result.data = to_type<int, float>(result)
                                   : throw BirdException("mismatching type in assignment, expected int");
            }
            else if (type_lexeme == "float")
            {
                is_numeric(result) ? result.data = to_type<float, int>(result)
                                   : throw BirdException("mismatching type in assignment, expected float");
            }
            else if (type_lexeme == "str" && !is_type<std::string>(result))
            {
                throw BirdException("mismatching type in assignment, expected str");
            }
            else if (type_lexeme == "bool" && !is_type<bool>(result))
            {
                throw BirdException("mismatching type in assignment, expected bool");
            }
        }

        this->environment->insert(const_stmt->identifier.lexeme, std::move(result));
    }

    void visit_while_stmt(WhileStmt *while_stmt)
    {
        auto original_environment = this->environment;

        while_stmt->condition->accept(this);
        auto condition_result = std::move(this->stack.top());
        this->stack.pop();

        if (!is_type<bool>(condition_result))
            throw BirdException("expected bool in while statement condition");

        while (as_type<bool>(condition_result))
        {
            try
            {
                while_stmt->stmt->accept(this);
            }
            catch (BreakException e)
            {
                break;
            }
            catch (ContinueException e)
            {
                while_stmt->condition->accept(this);
                condition_result = std::move(this->stack.top());
                this->stack.pop();

                this->environment = original_environment;

                continue;
            }

            while_stmt->condition->accept(this);
            condition_result = std::move(this->stack.top());
            this->stack.pop();
        }
    }

    void visit_for_stmt(ForStmt *for_stmt)
    {
        std::shared_ptr<SymbolTable<Value>> new_environment = std::make_shared<SymbolTable<Value>>();
        new_environment->set_enclosing(this->environment);
        this->environment = new_environment;

        if (for_stmt->initializer.has_value())
        {
            for_stmt->initializer.value()->accept(this);
        }

        while (true)
        {
            if (for_stmt->condition.has_value())
            {
                for_stmt->condition.value()->accept(this);
                auto condition_result = std::move(this->stack.top());
                this->stack.pop();

                if (!is_type<bool>(condition_result.data))
                {
                    throw BirdException("expected bool in for statement condition");
                }

                if (!as_type<bool>(condition_result.data))
                {
                    break;
                }
            }

            try
            {
                for_stmt->body->accept(this);
            }
            catch (BreakException e)
            {
                break;
            }
            catch (ContinueException e)
            {
                continue;
            }

            if (for_stmt->increment.has_value())
            {
                for_stmt->increment.value()->accept(this);
            }
        }

        this->environment = this->environment->get_enclosing();
    }

    void visit_binary(Binary *binary)
    {
        binary->left->accept(this);
        binary->right->accept(this);

        auto right = std::move(this->stack.top());
        this->stack.pop();

        auto left = std::move(this->stack.top());
        this->stack.pop();

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
            if ((is_type<int>(right) && as_type<int>(right) == 0) || (is_type<float>(right) && as_type<float>(right) == 0))
            {
                throw BirdException("Divide by 0 error.");
            }

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
        case Token::Type::PERCENT:
        {
            HANDLE_GENERAL_BINARY_OPERATOR(left, right, int, %);
            THROW_UNKNWOWN_BINARY_OPERATOR(%);
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
        auto expr = std::move(this->stack.top());
        this->stack.pop();

        if (is_type<int>(expr))
            this->stack.push(Value((-as_type<int>(expr))));
        else if (is_type<float>(expr))
            this->stack.push(Value(
                variant(-as_type<float>(expr))));
        else
            throw BirdException("Unknown type used with unary value.");
    }

    void visit_primary(Primary *primary)
    {
        switch (primary->value.token_type)
        {
        case Token::Type::FLOAT_LITERAL:
            this->stack.push(Value(
                variant(std::stof(primary->value.lexeme))));
            break;
        case Token::Type::BOOL_LITERAL:
            this->stack.push(Value(
                variant(primary->value.lexeme == "true" ? true : false)));
            break;
        case Token::Type::STR_LITERAL:
            this->stack.push(Value(
                variant(primary->value.lexeme)));
            break;
        case Token::Type::INT_LITERAL:
            this->stack.push(Value(
                variant(std::stoi(primary->value.lexeme))));
            break;
        case Token::Type::IDENTIFIER:
            this->stack.push(
                this->environment->get(primary->value.lexeme));
            break;
        default:
            throw BirdException("undefined primary value");
        }
    }

    void visit_ternary(Ternary *ternary)
    {
        ternary->condition->accept(this);

        auto result = std::move(this->stack.top());
        this->stack.pop();

        if (!is_type<bool>(result))
            throw BirdException("expected bool result for ternary condition");

        if (as_type<bool>(result))
            ternary->true_expr->accept(this);
        else
            ternary->false_expr->accept(this);
    }

    void visit_func(Func *func)
    {
        Callable callable = Callable(func->param_list,
                                     std::shared_ptr<Stmt>(
                                         std::move(func->block)),
                                     func->return_type);

        this->call_table->insert(func->identifier.lexeme, std::move(callable));
    }

    void visit_if_stmt(IfStmt *if_stmt)
    {
        if_stmt->condition->accept(this);

        auto result = std::move(this->stack.top());
        this->stack.pop();

        if (!is_type<bool>(result))
            throw BirdException("expected bool result for if-statement condition");

        if (as_type<bool>(result))
            if_stmt->then_branch->accept(this);
        else if (if_stmt->else_branch.has_value())
            if_stmt->else_branch.value()->accept(this);
    }

    void visit_call(Call *call)
    {
        if (!this->call_table->contains(call->identifier.lexeme))
        {
            throw BirdException("undefined function");
        }

        auto callable = this->call_table->get(call->identifier.lexeme);
        callable.call(this, std::move(call->args));
    }

    void visit_return_stmt(ReturnStmt *return_stmt)
    {
        if (return_stmt->expr.has_value())
        {
            return_stmt->expr.value()->accept(this);
        }

        throw ReturnException();
    }

    void visit_break_stmt(BreakStmt *break_stmt)
    {
        this->environment = this->environment->get_enclosing();
        throw BreakException();
    }

    void visit_continue_stmt(ContinueStmt *continue_stmt)
    {
        throw ContinueException();
    }
};