#pragma once

#include <memory>
#include "stmt.h"
#include "../../visitors/visitor.h"

// forward declaration
class Expr;

/*
 * if statement AST Node
 * ex:
 * if "(" expression ")" "{" statement "}"
 * else if "{" statment "}"
 * else "{" statement "}"
 */
class IfStmt : public Stmt
{
public:
    std::unique_ptr<Expr> condition;
    std::unique_ptr<Stmt> then_branch;
    std::unique_ptr<Stmt> else_branch;

    IfStmt(std::unique_ptr<Expr> condition, std::unique_ptr<Stmt> then_branch, std::unique_ptr<Stmt> else_branch)
    {
        this->condition = std::move(condition);
        this->then_branch = std::move(then_branch);
        this->else_branch = std::move(else_branch);
    }

    void accept(Visitor *visitor)
    {
        visitor->visit_if_stmt(this);
    }
};
