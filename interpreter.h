#pragma once

#include <memory>
#include <vector>
#include "expr.h"

#include "binary.h"
#include "unary.h"
#include "primary.h"
#include "sym_table.h"

class Interpreter : public Visitor
{
    SymbolTable environment;

public:
    void evaluate(Expr *expr)
    {
        if (auto *binary = dynamic_cast<Binary *>(expr))
        {
            binary->accept(this);
        }
        if (auto *unary = dynamic_cast<Unary *>(expr))
        {
            unary->accept(this);
        }
        if (auto *primary = dynamic_cast<Primary *>(expr))
        {
            primary->accept(this);
        }

        std::cout << this->stack[this->stack.size() - 1] << std::endl;
    }

    std::vector<int> stack;
    void visitBinary(Binary *binary)
    {
        binary->left->accept(this);
        binary->right->accept(this);

        auto left = this->stack[this->stack.size() - 1];
        this->stack.pop_back();

        auto right = this->stack[this->stack.size() - 1];
        this->stack.pop_back();

        switch (binary->op.token_type)
        {
        case TokenType::PLUS:
            this->stack.push_back(left + right);
            break;
        case TokenType::MINUS:
            this->stack.push_back(left - right);
            break;
        case TokenType::SLASH:
            this->stack.push_back(left / right);
            break;
        case TokenType::STAR:
            this->stack.push_back(left * right);
            break;
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
            this->stack.push_back(std::stoi(primary->value.lexeme));
            break;
        case TokenType::IDENTIFIER:
            auto value = this->environment.get(primary->value.lexeme);
            this->stack.push_back(value);
            break;
        }
    }
};
