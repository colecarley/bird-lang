#pragma once

#include <memory>
#include <optional>
#include "stmt.h"
#include "../../visitors/visitor.h"

// forward declaration
class Expr;

/*
 * for statement AST Node
 * ex:
 *  for Stmt | Expr ";" Expr ";" Expr {
 *      Stmt ";"
 *  }
 */
class ForStmt : public Stmt
{
public:
    std::optional<std::unique_ptr<Stmt>> initializer;
    std::optional<std::unique_ptr<Expr>> condition;
    std::optional<std::unique_ptr<Expr>> increment;
    std::unique_ptr<Stmt> body;

    ForStmt(std::optional<std::unique_ptr<Stmt>> initializer,
            std::optional<std::unique_ptr<Expr>> condition,
            std::optional<std::unique_ptr<Expr>> increment,
            std::unique_ptr<Stmt> body)
    {
        this->initializer = std::move(initializer);
        this->condition = std::move(condition);
        this->increment = std::move(increment);
        this->body = std::move(body);
    }

    void accept(Visitor *visitor)
    {
        visitor->visit_for_stmt(this);
    }
};