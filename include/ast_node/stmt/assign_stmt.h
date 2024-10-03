#pragma once

#include <string>
#include <memory>
#include "stmt.h"
#include "../../lexer.h"
#include "../../visitors/visitor.h"

// forward declaration
class Expr;

/*
 * Assignment statement AST Node that represents variable assignments
 * ex:
 * x = 4;
 */
class AssignStmt : public Stmt
{
public:
    Token identifier;
    std::unique_ptr<Expr> value;

    AssignStmt(Token identifier, std::unique_ptr<Expr> value)
    {
        this->identifier = identifier;
        this->value = std::move(value);
    }

    void accept(Visitor *visitor)
    {
        visitor->visit_assign_stmt(this);
    }
};
