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
    Token for_token;
    std::optional<std::unique_ptr<Stmt>> initializer;
    std::optional<std::unique_ptr<Expr>> condition;
    std::optional<std::unique_ptr<Expr>> increment;
    std::unique_ptr<Stmt> body;

    ForStmt(Token for_token,
            std::optional<std::unique_ptr<Stmt>> initializer,
            std::optional<std::unique_ptr<Expr>> condition,
            std::optional<std::unique_ptr<Expr>> increment,
            std::unique_ptr<Stmt> body)
        : for_token(for_token),
          initializer(std::move(initializer)),
          condition(std::move(condition)),
          increment(std::move(increment)),
          body(std::move(body)) {}

    ForStmt(Token for_token,
            std::optional<Stmt *> initializer,
            std::optional<Expr *> condition,
            std::optional<Expr *> increment,
            Stmt *body)
        : for_token(for_token),
          initializer(initializer),
          condition(condition),
          increment(increment),
          body(body) {}

    void accept(Visitor *visitor)
    {
        visitor->visit_for_stmt(this);
    }
};