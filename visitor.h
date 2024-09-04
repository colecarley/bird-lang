
#pragma once

#include <string>
#include <memory>

class Binary;
class Unary;
class Primary;
class DeclStmt;
class ExprStmt;
class PrintStmt;

class Visitor
{
public:
    ~Visitor() = default;
    virtual void visitBinary(Binary *) = 0;
    virtual void visitUnary(Unary *) = 0;
    virtual void visitPrimary(Primary *) = 0;
    virtual void visitDeclStmt(DeclStmt *) = 0;
    virtual void visitExprStmt(ExprStmt *) = 0;
    virtual void visitPrintStmt(PrintStmt *) = 0;
};
