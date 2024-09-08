#pragma once

#include <any>
#include <string>
#include "../node.h"

class Visitor;

class Expr : public Node
{
public:
    virtual ~Expr() = default;
    virtual void accept(Visitor *visitor) = 0;
};
