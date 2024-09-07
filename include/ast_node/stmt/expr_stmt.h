#pragma once

#include <memory>
#include "stmt.h"
#include "../../visitors/visitor.h"

class Expr;

class ExprStmt : public Stmt
{
public:
    std::unique_ptr<Expr> expr;

    ExprStmt(std::unique_ptr<Expr> expr)
    {
        this->expr = std::move(expr);
    }

    void accept(Visitor *visitor)
    {
        visitor->visit_expr_stmt(this);
    }
};
