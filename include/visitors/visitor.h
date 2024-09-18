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
class Block;
class ConstStmt;

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
    virtual void visit_block(Block *) = 0;
    virtual void visit_const_stmt(ConstStmt *) = 0;
    virtual void visit_if_stmt(IfStmt *) = 0;
};
