
#pragma once

#include <string>
#include <memory>

class Binary;
class Unary;
class Primary;

class Visitor
{
public:
    ~Visitor() = default;
    virtual void visitBinary(Binary *) = 0;
    virtual void visitUnary(Unary *) = 0;
    virtual void visitPrimary(Primary *) = 0;
};
