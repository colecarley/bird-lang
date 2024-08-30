#pragma once

#include <memory>
#include "lexer.h"
#include "visitor.h"
#include "expr.h"

class Unary : public Expr
{
public:
    Token op; // must be of type -
    std::unique_ptr<Expr> expr;

    Unary(Token op, std::unique_ptr<Expr> expr)
    {
        this->op = op;
        this->expr = std::move(expr);
    }

    void accept(Visitor *visitor)
    {
        return visitor->visitUnary(this);
    }
};
