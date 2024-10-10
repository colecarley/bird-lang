#pragma once

#include <memory>
#include <vector>
#include "../ast_node/stmt/stmt.h"
#include "../ast_node/expr/expr.h"

#include "../ast_node/expr/binary.h"
#include "../ast_node/expr/unary.h"
#include "../ast_node/expr/primary.h"
#include "../ast_node/expr/ternary.h"

#include "../ast_node/stmt/decl_stmt.h"
#include "../ast_node/stmt/assign_stmt.h"
#include "../ast_node/stmt/print_stmt.h"
#include "../ast_node/stmt/if_stmt.h"
#include "../ast_node/stmt/expr_stmt.h"
#include "../ast_node/stmt/while_stmt.h"
#include "../ast_node/stmt/for_stmt.h"
#include "../ast_node/stmt/block.h"

#include "../exceptions/bird_exception.h"

/*
 * Visitor that prints the Abstract Syntax Tree
 * using prefix notation and expressions wrapped in parentheses
 */
class AstPrinter : public Visitor
{
public:
    void print_ast(std::vector<std::unique_ptr<Stmt>> *stmts)
    {

        for (auto &stmt : *stmts)
        {
            if (auto decl_stmt = dynamic_cast<DeclStmt *>(stmt.get()))
            {
                decl_stmt->accept(this);
            }

            if (auto assign_stmt = dynamic_cast<AssignStmt *>(stmt.get()))
            {
                assign_stmt->accept(this);
            }

            if (auto print_stmt = dynamic_cast<PrintStmt *>(stmt.get()))
            {
                print_stmt->accept(this);
            }

            if (auto if_stmt = dynamic_cast<IfStmt *>(stmt.get()))
            {
                if_stmt->accept(this);
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

            if (auto for_stmt = dynamic_cast<ForStmt *>(stmt.get()))
            {
                for_stmt->accept(this);
            }

            std::cout << std::endl;
        }
    }

    void visit_block(Block *block)
    {
        std::cout << "{";
        for (auto &stmt : block->stmts)
        {
            stmt->accept(this);
        }
        std::cout << "}";
    }

    void visit_decl_stmt(DeclStmt *decl_stmt)
    {
        std::cout << "var ";
        std::cout << decl_stmt->identifier.lexeme << " = ";
        decl_stmt->value->accept(this);
    }

    void visit_assign_stmt(AssignStmt *assign_stmt)
    {
        std::cout << assign_stmt->identifier.lexeme << " " << assign_stmt->assign_operator.lexeme << " ";
        assign_stmt->value->accept(this);
    }

    void visit_print_stmt(PrintStmt *print_stmt)
    {
        std::cout << "print ";
        for (auto &arg : print_stmt->args)
        {
            arg->accept(this);
        }
    }

    void visit_expr_stmt(ExprStmt *expr_stmt)
    {
        expr_stmt->expr->accept(this);
    }

    void visit_binary(Binary *binary)
    {
        std::cout << "(";
        std::cout << binary->op.lexeme << " ";
        binary->left->accept(this);
        std::cout << " ";
        binary->right->accept(this);
        std::cout << ")";
    }

    void visit_unary(Unary *unary)
    {
        std::cout << unary->op.lexeme;
        unary->expr->accept(this);
    }

    void visit_primary(Primary *primary)
    {
        std::cout << primary->value.lexeme;
    }

    void visit_ternary(Ternary *ternary)
    {
        ternary->condition->accept(this);

        std::cout << " ? ";
        ternary->true_expr->accept(this);

        std::cout << " : ";
        ternary->false_expr->accept(this);
    }

    void visit_const_stmt(ConstStmt *const_stmt)
    {
        throw BirdException("implement const statment visit");
    }

    void visit_func(Func *func)
    {
        throw BirdException("implement func visit");
    }

    void visit_if_stmt(IfStmt *if_stmt)
    {
        std::cout << "if ";
        if_stmt->condition->accept(this);

        std::cout << " ";
        if_stmt->then_branch->accept(this);

        if (if_stmt->else_branch.has_value())
        {
            std::cout << " else ";
            if_stmt->else_branch.value()->accept(this);
        }
    }

    void visit_while_stmt(WhileStmt *while_stmt)
    {
        std::cout << "while (";
        while_stmt->condition->accept(this);
        std::cout << ") ";
        while_stmt->stmt->accept(this);
    }

    void visit_for_stmt(ForStmt *for_stmt)
    {
        std::cout << "for (";
        if (for_stmt->initializer.has_value())
        {
            for_stmt->initializer.value()->accept(this);
        }
        std::cout << "; ";
        if (for_stmt->condition.has_value())
        {
            for_stmt->condition.value()->accept(this);
        }
        std::cout << "; ";

        if (for_stmt->increment.has_value())
        {
            for_stmt->increment.value()->accept(this);
        }
        std::cout << ") ";
        for_stmt->body->accept(this);
    }
};
