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
 *  for Stmt | Expr ";" Expr ";" Stmt {
 *      Stmt ";"
 *  }
 */
class ForStmt : public Stmt
{
public:
    std::optional<std::unique_ptr<Stmt>> initializer;
    std::optional<std::unique_ptr<Expr>> condition;
    std::optional<std::unique_ptr<Stmt>> increment;
    std::unique_ptr<Stmt> body;

    ForStmt(std::optional<std::unique_ptr<Stmt>> initializer,
            std::optional<std::unique_ptr<Expr>> condition,
            std::optional<std::unique_ptr<Stmt>> increment,
            std::unique_ptr<Stmt> body)
        : initializer(std::move(initializer)),
          condition(std::move(condition)),
          increment(std::move(increment)),
          body(std::move(body)) {}

    void accept(Visitor *visitor)
    {
        visitor->visit_for_stmt(this);
    }
};