#pragma once

#include <memory>
#include <vector>
#include <variant>
#include <iostream>

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
#include "../ast_node/stmt/type_stmt.h"

#include "../sym_table.h"
#include "../exceptions/bird_exception.h"
#include "../exceptions/return_exception.h"
#include "../exceptions/break_exception.h"
#include "../exceptions/continue_exception.h"
#include "../value.h"
#include "../callable.h"
#include "../stack.h"
#include "../type.h"

/*
 * Visitor that interprets and evaluates the AST
 */
class Interpreter : public Visitor
{

public:
    Environment<Value> env;
    Environment<Callable> call_table;
    Environment<Type> type_table;
    Stack<Value> stack;

    Interpreter()
    {
        this->env.push_env();
        this->call_table.push_env();
        this->type_table.push_env();
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
                continue;
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

            if (auto type_stmt = dynamic_cast<TypeStmt *>(stmt.get()))
            {
                type_stmt->accept(this);
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
        this->env.push_env();
        for (auto &stmt : block->stmts)
        {
            stmt->accept(this);
        }

        this->env.pop_env();
    }

    void visit_decl_stmt(DeclStmt *decl_stmt)
    {
        decl_stmt->value->accept(this);

        auto result = std::move(this->stack.pop());
        result.is_mutable = true;

        if (decl_stmt->type_token.has_value())
        {
            std::string type_lexeme;
            if (decl_stmt->type_is_literal)
            {
                type_lexeme = decl_stmt->type_token.value().lexeme;
            }
            else
            {
                type_lexeme = this->type_table.get(decl_stmt->type_token.value().lexeme).type.lexeme;
            }

            if (type_lexeme == "int")
            {
                result.data = to_type<int, double>(result);
            }
            else if (type_lexeme == "float")
            {
                result.data = to_type<double, int>(result);
            }
        }

        this->env.declare(decl_stmt->identifier.lexeme, std::move(result));
    }

    void visit_assign_expr(AssignExpr *assign_expr)
    {
        auto previous_value = this->env.get(assign_expr->identifier.lexeme);

        assign_expr->value->accept(this);
        auto value = std::move(this->stack.pop());

        switch (assign_expr->assign_operator.token_type)
        {
        case Token::Type::EQUAL:
        {
            previous_value = value;
            break;
        }
        case Token::Type::PLUS_EQUAL:
        {
            previous_value = previous_value + value;
            break;
        }
        case Token::Type::MINUS_EQUAL:
        {
            previous_value = previous_value - value;
            break;
        }
        case Token::Type::STAR_EQUAL:
        {
            previous_value = previous_value * value;
            break;
        }
        case Token::Type::SLASH_EQUAL:
        {
            previous_value = previous_value / value;
            break;
        }
        case Token::Type::PERCENT_EQUAL:
        {
            previous_value = previous_value % value;
            break;
        }
        default:
            throw BirdException("Unidentified assignment operator " + assign_expr->assign_operator.lexeme);
        }

        this->env.set(assign_expr->identifier.lexeme, previous_value);
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
            auto result = std::move(this->stack.pop());

            std::cout << result;
        }
        std::cout << std::endl;
    }

    void visit_const_stmt(ConstStmt *const_stmt)
    {
        const_stmt->value->accept(this);

        auto result = std::move(this->stack.pop());

        if (const_stmt->type_token.has_value())
        {
            std::string type_lexeme;
            if (const_stmt->type_is_literal)
            {
                type_lexeme = const_stmt->type_token.value().lexeme;
            }
            else
            {
                type_lexeme = this->type_table.get(const_stmt->type_token.value().lexeme).type.lexeme;
            }

            if (type_lexeme == "int")
            {
                result.data = to_type<int, double>(result);
            }
            else if (type_lexeme == "float")
            {
                result.data = to_type<double, int>(result);
            }
        }

        this->env.declare(const_stmt->identifier.lexeme, std::move(result));
    }

    void visit_while_stmt(WhileStmt *while_stmt)
    {
        while_stmt->condition->accept(this);
        auto condition_result = std::move(this->stack.pop());

        auto num_envs = this->env.envs.size();

        while (as_type<bool>(condition_result))
        {
            try
            {
                while_stmt->stmt->accept(this);
            }
            catch (BreakException e)
            {
                auto previous_size = this->env.envs.size();
                for (int i = 0; i < previous_size - num_envs; i++)
                {
                    this->env.pop_env();
                }

                break;
            }
            catch (ContinueException e)
            {
                auto previous_size = this->env.envs.size();
                for (int i = 0; i < previous_size - num_envs; i++)
                {
                    this->env.pop_env();
                }

                while_stmt->condition->accept(this);
                condition_result = std::move(this->stack.pop());
                continue;
            }

            while_stmt->condition->accept(this);
            condition_result = std::move(this->stack.pop());
        }
    }

    void visit_for_stmt(ForStmt *for_stmt)
    {
        this->env.push_env();

        if (for_stmt->initializer.has_value())
        {
            for_stmt->initializer.value()->accept(this);
        }

        auto num_envs = this->env.envs.size();

        while (true)
        {
            if (for_stmt->condition.has_value())
            {
                for_stmt->condition.value()->accept(this);
                auto condition_result = std::move(this->stack.pop());

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
                auto previous_size = this->env.envs.size();
                for (int i = 0; i < previous_size - num_envs; i++)
                {
                    this->env.pop_env();
                }

                break;
            }
            catch (ContinueException e)
            {
                auto previous_size = this->env.envs.size();
                for (int i = 0; i < previous_size - num_envs; i++)
                {
                    this->env.pop_env();
                }

                if (for_stmt->increment.has_value())
                {
                    for_stmt->increment.value()->accept(this);
                }
                continue;
            }

            if (for_stmt->increment.has_value())
            {
                for_stmt->increment.value()->accept(this);
            }
        }

        this->env.pop_env();
    }

    void visit_binary(Binary *binary)
    {
        binary->left->accept(this);
        binary->right->accept(this);

        auto right = std::move(this->stack.pop());

        auto left = std::move(this->stack.pop());

        switch (binary->op.token_type)
        {
        case Token::Type::PLUS:
        {
            this->stack.push(left + right);
            break;
        }
        case Token::Type::MINUS:
        {
            this->stack.push(left - right);
            break;
        }
        case Token::Type::SLASH:
        {
            this->stack.push(left / right);
            break;
        }
        case Token::Type::STAR:
        {
            this->stack.push(left * right);
            break;
        }
        case Token::Type::GREATER:
        {
            this->stack.push(left > right);
            break;
        }
        case Token::Type::GREATER_EQUAL:
        {
            this->stack.push(left >= right);
            break;
        }
        case Token::Type::LESS:
        {
            this->stack.push(left < right);
            break;
        }
        case Token::Type::LESS_EQUAL:
        {
            this->stack.push(left <= right);
            break;
        }
        case Token::Type::BANG_EQUAL:
        {
            this->stack.push(left != right);
            break;
        }
        case Token::Type::EQUAL_EQUAL:
        {
            this->stack.push(left == right);
            break;
        }
        case Token::Type::PERCENT:
        {
            this->stack.push(left % right);
            break;
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
        auto expr = std::move(this->stack.pop());

        this->stack.push(-expr);
    }

    void visit_primary(Primary *primary)
    {
        switch (primary->value.token_type)
        {
        case Token::Type::FLOAT_LITERAL:
            this->stack.push(Value(
                variant(std::stod(primary->value.lexeme))));
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
                this->env.get(primary->value.lexeme));
            break;
        default:
            throw BirdException("undefined primary value");
        }
    }

    void visit_ternary(Ternary *ternary)
    {
        ternary->condition->accept(this);

        auto result = std::move(this->stack.pop());

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

        this->call_table.declare(func->identifier.lexeme, std::move(callable));
    }

    void visit_if_stmt(IfStmt *if_stmt)
    {
        if_stmt->condition->accept(this);

        auto result = std::move(this->stack.pop());

        if (as_type<bool>(result))
            if_stmt->then_branch->accept(this);
        else if (if_stmt->else_branch.has_value())
            if_stmt->else_branch.value()->accept(this);
    }

    void visit_call(Call *call)
    {
        auto callable = this->call_table.get(call->identifier.lexeme);
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
        throw BreakException();
    }

    void visit_continue_stmt(ContinueStmt *continue_stmt)
    {
        throw ContinueException();
    }

    void visit_type_stmt(TypeStmt *type_stmt)
    {
        if (type_stmt->type_is_literal)
        {
            this->type_table.declare(type_stmt->identifier.lexeme, Type(type_stmt->type_token));
        }
        else
        {
            this->type_table.declare(type_stmt->identifier.lexeme, Type(this->type_table.get(type_stmt->type_token.lexeme).type));
        }
    }
};