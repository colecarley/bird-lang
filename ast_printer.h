#pragma once

#include <memory>
#include "expr.h"

#include "binary.h"
#include "unary.h"
#include "primary.h"

class AstPrinter : public Visitor
{
public:
    void print_ast(Expr *expr)
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
        std::cout << std::endl;
    }

    void visitBinary(Binary *binary)
    {
        std::cout << "(";
        binary->left->accept(this);
        std::cout << binary->op.lexeme;
        binary->right->accept(this);
        std::cout << ")";
    }

    void visitUnary(Unary *unary)
    {
        std::cout << unary->op.lexeme;
        unary->expr->accept(this);
    }

    void visitPrimary(Primary *primary)
    {
        std::cout << primary->value.lexeme;
    }
};
