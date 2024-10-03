#pragma once

#include <memory>
#include <optional>
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
 *
 * or
 *
 * ex:
 * if expression
 *       statement
 * else
 *       statment
 */
class IfStmt : public Stmt
{
public:
    std::unique_ptr<Expr> condition;
    std::unique_ptr<Stmt> then_branch;
    std::optional<std::unique_ptr<Stmt>> else_branch;

    IfStmt(std::unique_ptr<Expr> condition,
           std::unique_ptr<Stmt> then_branch,
           std::optional<std::unique_ptr<Stmt>> else_branch)
    {
        this->condition = std::move(condition);
        this->then_branch = std::move(then_branch);

        if (else_branch.has_value())
        {
            this->else_branch = std::move(else_branch);
        }
        else
        {
            this->else_branch = std::nullopt;
        }
    }

    void accept(Visitor *visitor)
    {
        visitor->visit_if_stmt(this);
    }
};
