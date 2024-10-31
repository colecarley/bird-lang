#pragma once

#include "stmt.h"
#include "../../visitors/visitor.h"

/*
 * Break statement class AST node that
 * represents loop breaks
 * ex:
 * while (true) {
 *     break;
 * }
 */
class BreakStmt : public Stmt
{
public:
    BreakStmt() {}

    void accept(Visitor *visitor)
    {
        visitor->visit_break_stmt(this);
    }
};