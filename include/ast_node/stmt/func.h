#pragma once

#include <string>
#include <memory>
#include <vector>
#include "stmt.h"
#include "../../lexer.h"
#include "../../visitors/visitor.h"

// forward declaration
class Visitor;

/*
 * Function definition AST Node
 *
 * fn foobar(foo: int, bar: int) -> int {
 *    statement;
 *    statement;
 *    statement;
 * }
 */
class Func : public Stmt
{
public:
    Token identifier;
    std::optional<Token> return_type;
    // the first item in the pair is an identifier, the second is a type
    std::vector<std::pair<Token, Token>> param_list; // TODO: make this an actual type
    std::unique_ptr<Stmt> block;

    Func(Token identifier,
         std::optional<Token> return_type,
         std::vector<std::pair<Token, Token>> param_list,
         std::unique_ptr<Stmt> block) : identifier(identifier),
                                        return_type(return_type),
                                        param_list(param_list),
                                        block(std::move(block)) {}

    void accept(Visitor *visitor)
    {
        visitor->visit_func(this);
    }
};
