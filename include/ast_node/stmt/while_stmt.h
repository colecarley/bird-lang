#pragma once

#include <memory>
#include "stmt.h"
#include "../../visitors/visitor.h"

// forward declaration
class Expr;

/*
 * while statement AST Node
 * ex:
 * while %expression% %statement%
 */
class WhileStmt : public Stmt
{
public:
    std::unique_ptr<Expr> condition;
    std::unique_ptr<Stmt> stmt;

    WhileStmt(std::unique_ptr<Expr> condition, std::unique_ptr<Stmt> stmt)
    {
        this->condition = std::move(condition);
        this->stmt = std::move(stmt);
    }

    void accept(Visitor *visitor)
    {
        visitor->visit_while_stmt(this);
    }
};