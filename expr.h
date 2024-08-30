#pragma once

#include <any>
#include <string>

class Visitor;

class Expr
{
public:
    virtual ~Expr() = default;
    virtual void accept(Visitor *visitor) = 0;
};
