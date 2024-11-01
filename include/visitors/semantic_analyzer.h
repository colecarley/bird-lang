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
#include "../exceptions/user_error_tracker.h"
#include "../value.h"
#include "../bird_type.h"

/*
 * Visitor that analyzes semantics of the AST
 */
class SemanticAnalyzer : public Visitor
{

public:
    std::shared_ptr<SymbolTable<Value>> environment;
    std::shared_ptr<SymbolTable<BirdFunction>> call_table;
    UserErrorTracker *user_error_tracker;
    int loop_depth;
    int function_depth;

    SemanticAnalyzer(UserErrorTracker *user_error_tracker) : user_error_tracker(user_error_tracker)
    {
        this->environment = std::make_shared<SymbolTable<Value>>();
        this->call_table = std::make_shared<SymbolTable<BirdFunction>>();
        this->loop_depth = 0;
        this->function_depth = 0;
    }

    void analyze_semantics(std::vector<std::unique_ptr<Stmt>> *stmts)
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
                this->user_error_tracker->semantic_error("Identifier '" + decl_stmt->identifier.lexeme + "' is already declared.");
                return;
            }

            current_env = current_env->get_enclosing();
        }

        decl_stmt->value->accept(this);

        Value mutable_value;
        mutable_value.is_mutable = true;
        this->environment->insert(decl_stmt->identifier.lexeme, mutable_value);
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
            this->user_error_tracker->semantic_error("Identifier '" + assign_expr->identifier.lexeme + "' is not initialized.");
            return;
        }

        auto previous_value = current_env->get(assign_expr->identifier.lexeme);

        if (!previous_value.is_mutable)
        {
            this->user_error_tracker->semantic_error("Identifier '" + assign_expr->identifier.lexeme + "' is not mutable.");
            return;
        }

        assign_expr->value->accept(this);
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
        }
    }

    void visit_const_stmt(ConstStmt *const_stmt)
    {
        std::shared_ptr<SymbolTable<Value>> current_env = this->environment;

        while (current_env)
        {
            if (this->environment->contains(const_stmt->identifier.lexeme))
            {
                this->user_error_tracker->semantic_error("Identifier '" + const_stmt->identifier.lexeme + "' is already declared.");
                return;
            }

            current_env = current_env->get_enclosing();
        }

        const_stmt->value->accept(this);

        this->environment->insert(const_stmt->identifier.lexeme, Value());
    }

    void visit_while_stmt(WhileStmt *while_stmt)
    {
        this->loop_depth += 1;

        while_stmt->condition->accept(this);
        while_stmt->stmt->accept(this);

        this->loop_depth -= 1;
    }

    void visit_for_stmt(ForStmt *for_stmt)
    {
        this->loop_depth += 1;

        std::shared_ptr<SymbolTable<Value>> new_environment = std::make_shared<SymbolTable<Value>>();
        new_environment->set_enclosing(this->environment);
        this->environment = new_environment;

        if (for_stmt->initializer.has_value())
        {
            for_stmt->initializer.value()->accept(this);
        }

        if (for_stmt->condition.has_value())
        {
            for_stmt->condition.value()->accept(this);
        }

        for_stmt->body->accept(this);

        if (for_stmt->increment.has_value())
        {
            for_stmt->increment.value()->accept(this);
        }

        this->environment = this->environment->get_enclosing();

        this->loop_depth -= 1;
    }

    void visit_binary(Binary *binary)
    {
        binary->left->accept(this);
        binary->right->accept(this);
    }

    void visit_unary(Unary *unary)
    {
        unary->expr->accept(this);
    }

    void visit_primary(Primary *primary)
    {
        if (primary->value.token_type == Token::Type::IDENTIFIER && !this->environment->is_accessible(primary->value.lexeme))
        {
            this->user_error_tracker->semantic_error("Identifier '" + primary->value.lexeme + "' is not initialized.");
            return;
        }
    }

    void visit_ternary(Ternary *ternary)
    {
        ternary->condition->accept(this);
        ternary->true_expr->accept(this);
        ternary->false_expr->accept(this);
    }

    void visit_func(Func *func)
    {
        this->function_depth += 1;

        this->call_table->insert(func->identifier.lexeme, BirdFunction());

        this->function_depth -= 1;
    }

    void visit_if_stmt(IfStmt *if_stmt)
    {
        if_stmt->condition->accept(this);
        if_stmt->then_branch->accept(this);
        
        if (if_stmt->else_branch.has_value())
        {
            if_stmt->else_branch.value()->accept(this);
        }
    }

    void visit_call(Call *call)
    {
        if (!this->call_table->contains(call->identifier.lexeme))
        {
            this->user_error_tracker->semantic_error("Function call identifier '" + call->identifier.lexeme + "' is not declared.");
            return;
        }

        auto function = this->call_table->get(call->identifier.lexeme);

        if (function.params.size() != call->args.size())
        {
            this->user_error_tracker->semantic_error("Function call identifer '" + call->identifier.lexeme + "' does not use the correct number of arguments.");
            return;
        }
    }

    void visit_return_stmt(ReturnStmt *return_stmt)
    {
        if (this->function_depth == 0)
        {
            this->user_error_tracker->semantic_error("Return statement is declared outside of a function.");
            return;
        }

        if (return_stmt->expr.has_value())
        {
            return_stmt->expr.value()->accept(this);
        }
    }

    void visit_break_stmt(BreakStmt *break_stmt)
    {
        if (this->loop_depth == 0)
        {
            this->user_error_tracker->semantic_error("Break statement is declared outside of a loop.");
            return;
        }
    }

    void visit_continue_stmt(ContinueStmt *continue_stmt)
    {
        if (this->loop_depth == 0)
        {
            this->user_error_tracker->semantic_error("Continue statement is declared outside of a loop.");
            return;
        }
    }
};