#pragma once

#include <string>
#include <memory>
#include "stmt.h"
#include "../../lexer.h"
#include "../../visitors/visitor.h"

class Expr;

class DeclStmt : public Stmt
{
public:
    Token identifier;
    std::unique_ptr<Expr> value;

    DeclStmt(Token identifier, std::unique_ptr<Expr> value)
    {
        this->identifier = identifier;
        this->value = std::move(value);
    }

    void accept(Visitor *visitor)
    {
        visitor->visit_decl_stmt(this);
    }
};
