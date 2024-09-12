#pragma once

#include <memory>
#include <vector>

#include "../ast_node/stmt/stmt.h"
#include "../ast_node/expr/expr.h"

#include "../ast_node/expr/binary.h"
#include "../ast_node/expr/unary.h"
#include "../ast_node/expr/primary.h"

#include "../ast_node/stmt/decl_stmt.h"
#include "../ast_node/stmt/expr_stmt.h"
#include "../ast_node/stmt/print_stmt.h"
#include "../ast_node/stmt/block.h"

#include "../sym_table.h"
#include "../exceptions/bird_exception.h"

/*
 * Visitor that interprets and evaluates the AST
 */
class Interpreter : public Visitor
{
    std::unique_ptr<SymbolTable<int>> environment;
    std::vector<int> stack;

public:
    Interpreter()
    {
        this->environment = std::make_unique<SymbolTable<int>>(SymbolTable<int>());
    }

    void evaluate(std::vector<std::unique_ptr<Stmt>> *stmts)
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
        }
        this->stack.clear();
    }

    void visit_block(Block *block)
    {
        auto new_environment = std::make_unique<SymbolTable<int>>(SymbolTable<int>());
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

        auto result = this->stack[this->stack.size() - 1];
        this->stack.pop_back();

        this->environment->insert(decl_stmt->identifier.lexeme, result);
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

            std::cout << result;
        }
        std::cout << std::endl;
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
        case TokenType::PLUS:
        {
            this->stack.push_back(left + right);
            break;
        }
        case TokenType::MINUS:
        {
            this->stack.push_back(left - right);
            break;
        }
        case TokenType::SLASH:
        {
            this->stack.push_back(left / right);
            break;
        }
        case TokenType::STAR:
        {
            this->stack.push_back(left * right);
            break;
        }
        default:
        {
            throw BirdException("undefined binary operator");
        }
        }
    }

    void visit_unary(Unary *unary)
    {
        unary->expr->accept(this);
        auto expr = this->stack[this->stack.size() - 1];
        this->stack.pop_back();

        this->stack.push_back(-expr);
    }

    void visit_primary(Primary *primary)
    {
        switch (primary->value.token_type)
        {
        case TokenType::I32_LITERAL:
        {
            this->stack.push_back(std::stoi(primary->value.lexeme));
            break;
        }
        case TokenType::IDENTIFIER:
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
};
