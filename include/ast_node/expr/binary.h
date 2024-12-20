#pragma once

#include <memory>

#include "lexer.h"
#include "visitors/visitor.h"
#include "expr.h"

/*
 *
 * binary class AST Node representing binary operations
 * ex:
 * 1 + 1,
 * 1 >= 1
 */
class Binary : public Expr
{
public:
    std::unique_ptr<Expr> left;
    Token op;
    std::unique_ptr<Expr> right;

    Binary(std::unique_ptr<Expr> left, Token op, std::unique_ptr<Expr> right)
    {
        this->left = std::move(left);
        this->op = op;
        this->right = std::move(right);
    }

    void accept(Visitor *visitor)
    {
        return visitor->visit_binary(this);
    }
};
