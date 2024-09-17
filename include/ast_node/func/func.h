#pragma once

#include <string>
#include <memory>
#include <vector>
#include "../node.h"
#include "../../lexer.h"
#include "../../visitors/visitor.h"


// forward declaration
class Visitor;
class Stmt;

/*
 * Function definition AST Node
 * 
 * fn foobar(foo: int, bar: int): int {
 *    statement;
 *    statement;
 *    statement;
 * }
 */
class Func : public Node
{
public:
    Token identifier;
    std::unique_ptr<ParamList> param_list;
    std::unique_ptr<ReturnType> return_type;
    std::unique_ptr<Block> block;



    ~Func() = default;
    void accept(Visitor *visitor)
    {
        visitor->visit_func(this);
    }
};
