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
 * x += 4;
 */
class AssignStmt : public Stmt
{
public:
    Token identifier;
    Token assign_operator;
    std::unique_ptr<Expr> value;

    AssignStmt(Token identifier, Token assign_operator, std::unique_ptr<Expr> value)
    {
        this->identifier = identifier;
        this->assign_operator = assign_operator;
        this->value = std::move(value);
    }

    void accept(Visitor *visitor)
    {
        visitor->visit_assign_stmt(this);
    }
};
