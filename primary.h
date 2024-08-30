#pragma once

#include "lexer.h"
#include "visitor.h"
#include "expr.h"

class Primary : public Expr
{
public:
    Token value; // must be i32 literal
    Primary(Token value)
    {
        this->value = value;
    }

    void accept(Visitor *visitor)
    {
        return visitor->visitPrimary(this);
    }
};
