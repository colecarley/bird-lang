#pragma once

class Visitor;

class Node
{
public:
    virtual void accept(Visitor *visitor) = 0;
};
