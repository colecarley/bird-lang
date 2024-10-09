#pragma once

#include <vector>
#include <memory>
#include <optional>

#include "./lexer.h"

class Stmt;
class Expr;
class Interpreter;

class Callable
{
    std::vector<std::pair<Token, Token>> param_list;
    std::unique_ptr<Stmt> block;
    std::optional<Token> return_type;

public:
    Callable(
        std::vector<std::pair<Token, Token>> param_list,
        std::unique_ptr<Stmt> block,
        std::optional<Token> return_type)
        : param_list(param_list),
          block(std::move(block)),
          return_type(return_type) {}
    Callable() = default;

    void call(Interpreter *Interpreter, std::vector<std::unique_ptr<Expr>>);
};