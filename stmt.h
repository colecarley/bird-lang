#pragma once

#include "node.h"

class Visitor;

class Stmt : public Node
{
public:
    virtual ~Stmt() = default;
    virtual void accept(Visitor *visitor) = 0;
};
