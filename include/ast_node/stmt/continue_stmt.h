#pragma once

#include "stmt.h"
#include "../../visitors/visitor.h"

/*
 * Continue statement class AST node that
 * represents loop continue
 * ex:
 * while (true) {
 *     continue;
 * }
 */
class ContinueStmt : public Stmt
{
public:
    ContinueStmt() {}

    void accept(Visitor *visitor)
    {
        visitor->visit_continue_stmt(this);
    }
};