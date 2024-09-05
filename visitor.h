
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
    virtual void visit_binary(Binary *) = 0;
    virtual void visit_unary(Unary *) = 0;
    virtual void visit_primary(Primary *) = 0;
    virtual void visit_decl_stmt(DeclStmt *) = 0;
    virtual void visit_expr_stmt(ExprStmt *) = 0;
    virtual void visit_print_stmt(PrintStmt *) = 0;
};
