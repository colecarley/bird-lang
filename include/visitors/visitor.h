#pragma once

#include <string>
#include <memory>

// forward declarations
class Binary;
class Unary;
class Primary;
class DeclStmt;
class ExprStmt;
class PrintStmt;

/*
 * the interface for all visitors,
 * used through the AST nodes' `accept` function
 */
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
