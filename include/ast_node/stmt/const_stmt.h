#pragma once

#include <string>
#include <memory>
#include "stmt.h"
#include "../../token.h"
#include "../../visitors/visitor.h"

// forward declaration
class Expr;

/*
 * Const Declaration statement AST Node that represents variable declarations
 * ex:
 * const x: int = 4;
 */
class ConstStmt : public Stmt
{
public:
    Token identifier;
    std::optional<Token> type_identifier;
    std::unique_ptr<Expr> value;

    ConstStmt(Token identifier, std::optional<Token> type_identifier, std::unique_ptr<Expr> value)
    {
        this->identifier = identifier;
        this->type_identifier = type_identifier;
        this->value = std::move(value);
    }

    void accept(Visitor *visitor)
    {
        visitor->visit_const_stmt(this);
    }
};
