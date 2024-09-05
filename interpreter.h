#pragma once

#include <memory>
#include <vector>
#include "stmt.h"
#include "expr.h"

#include "binary.h"
#include "unary.h"
#include "primary.h"

#include "decl_stmt.h"
#include "print_stmt.h"
#include "expr_stmt.h"

#include "sym_table.h"
#include "bird_exception.h"

class Interpreter : public Visitor
{
    SymbolTable<int> environment;
    std::vector<int> stack;

public:
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

            if (auto expr_stmt = dynamic_cast<ExprStmt *>(stmt.get()))
            {
                expr_stmt->accept(this);
            }
        }
        this->stack.clear();
    }

    void visitDeclStmt(DeclStmt *decl_stmt)
    {
        decl_stmt->value->accept(this);

        auto result = this->stack[this->stack.size() - 1];
        this->stack.pop_back();

        this->environment.insert(decl_stmt->identifier.lexeme, result);
    }

    void visitExprStmt(ExprStmt *expr_stmt)
    {
        expr_stmt->expr->accept(this);
    }

    void visitPrintStmt(PrintStmt *print_stmt)
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

    void visitBinary(Binary *binary)
    {
        binary->left->accept(this);
        binary->right->accept(this);

        auto right = this->stack[this->stack.size() - 1];
        this->stack.pop_back();

        auto left = this->stack[this->stack.size() - 1];
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

    void visitUnary(Unary *unary)
    {
        unary->expr->accept(this);
        auto expr = this->stack[this->stack.size() - 1];
        this->stack.pop_back();

        this->stack.push_back(-expr);
    }

    void visitPrimary(Primary *primary)
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
            auto value = this->environment.get(primary->value.lexeme);
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
