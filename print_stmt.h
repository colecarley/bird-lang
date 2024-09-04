#pragma once

#include <string>
#include <memory>
#include <vector>

#include "stmt.h"
#include "lexer.h"
#include "visitor.h"

class Expr;

class PrintStmt : public Stmt
{
public:
    std::vector<std::unique_ptr<Expr>> args;

    PrintStmt(std::vector<std::unique_ptr<Expr>> args)
    {
        this->args = std::move(args);
    }

    void accept(Visitor *visitor)
    {
        visitor->visitPrintStmt(this);
    }
};
