#pragma once

#include <memory>
#include <vector>
#include "../ast_node/stmt/stmt.h"
#include "../ast_node/expr/expr.h"

#include "../ast_node/expr/binary.h"
#include "../ast_node/expr/unary.h"
#include "../ast_node/expr/primary.h"

#include "../ast_node/stmt/decl_stmt.h"
#include "../ast_node/stmt/print_stmt.h"
#include "../ast_node/stmt/expr_stmt.h"
#include "../ast_node/stmt/while_stmt.h"
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

    void visit_const_stmt(ConstStmt *const_stmt)
    {
        throw BirdException("implement const statment visit");
    }

    void visit_func(Func *func)
    {
        throw BirdException("implement func visit");
    }

    void visit_while_stmt(WhileStmt *while_stmt)
    {
        std::cout << "while (";
        while_stmt->condition->accept(this);
        std::cout << ") ";
        while_stmt->stmt->accept(this);
    }
};
