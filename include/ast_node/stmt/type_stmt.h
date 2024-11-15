#pragma once

#include <string>
#include <memory>
#include <optional>

#include "stmt.h"
#include "../../lexer.h"
#include "../../visitors/visitor.h"

// forward declaration
class Expr;

/*
 * Type statement AST Node that represents type declarations
 * ex:
 * type x = int;
 */
class TypeStmt : public Stmt
{
public:
    Token type_identifier;
    Token type_token;
    bool type_is_literal;

    TypeStmt(Token type_identifier, Token type_token, bool type_is_literal)
    {
        this->type_identifier = type_identifier;
        this->type_token = type_token;
        this->type_is_literal = type_is_literal;
    }

    void accept(Visitor *visitor)
    {
        visitor->visit_type_stmt(this);
    }
};