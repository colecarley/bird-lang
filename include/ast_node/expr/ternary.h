#pragma once

#include <memory>
#include "../../lexer.h"
#include "../../visitors/visitor.h"
#include "expr.h"

/*
 * Ternary class AST node representing ternary operations
 * ex:
 * ( x > y) ? print 1 : print 2;
 */
class Ternary : public Expr
{
public:
    std::unique_ptr<Expr> condition;
    std::unique_ptr<Expr> true_expr;
    std::unique_ptr<Expr> false_expr;

    Ternary(std::unique_ptr<Expr> condition,
            std::unique_ptr<Expr> true_expr,
            std::unique_ptr<Expr> false_expr)
    {
        this->condition = std::move(condition);
        this->true_expr = std::move(true_expr);
        this->false_expr = std::move(false_expr);
    }

    void accept(Visitor *visitor)
    {
        return visitor->visit_ternary(this);
    }
};
