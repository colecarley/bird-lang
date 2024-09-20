#pragma once

#include <memory>
#include <vector>
#include <variant>

#include "../ast_node/stmt/stmt.h"
#include "../ast_node/expr/expr.h"

#include "../ast_node/expr/binary.h"
#include "../ast_node/expr/unary.h"
#include "../ast_node/expr/primary.h"

#include "../ast_node/stmt/decl_stmt.h"
#include "../ast_node/stmt/expr_stmt.h"
#include "../ast_node/stmt/print_stmt.h"
#include "../ast_node/stmt/const_stmt.h"
#include "../ast_node/stmt/while_stmt.h"
#include "../ast_node/stmt/if_stmt.h"
#include "../ast_node/stmt/block.h"

#include "../sym_table.h"
#include "../exceptions/bird_exception.h"

enum DataType
{
    INT,
    FLOAT,
    STR,
    BOOL,
};

using variant = std::variant<int, float, std::string, bool>;

class Value
{
public:
    DataType data_type;
    std::variant<int, float, std::string, bool> data;
    bool is_mutable;

    Value(DataType data_type, variant data, bool is_mutable = false)
    {
        this->is_mutable = is_mutable;
        this->data = data;
        this->data_type = data_type;
    }

    Value() {};
};

/*
 * Visitor that interprets and evaluates the AST
 */
class Interpreter : public Visitor
{
    std::unique_ptr<SymbolTable<Value>> environment;
    std::vector<Value> stack;

public:
    Interpreter()
    {
        this->environment = std::make_unique<SymbolTable<Value>>(SymbolTable<Value>());
    }

    void evaluate(std::vector<std::unique_ptr<Stmt>> *stmts)
    {
        for (auto &stmt : *stmts)
        {
            if (auto decl_stmt = dynamic_cast<DeclStmt *>(stmt.get()))
            {
                decl_stmt->accept(this);
            }

            if (auto print_stmt = dynamic_cast<PrintStmt *>(stmt.get()))
            {
                print_stmt->accept(this);
            }

            if (auto block = dynamic_cast<Block *>(stmt.get()))
            {
                block->accept(this);
            }

            if (auto expr_stmt = dynamic_cast<ExprStmt *>(stmt.get()))
            {
                expr_stmt->accept(this);
            }

            if (auto while_stmt = dynamic_cast<WhileStmt *>(stmt.get()))
            {
                while_stmt->accept(this);
            }
        }
        this->stack.clear();
    }

    void visit_block(Block *block)
    {
        auto new_environment = std::make_unique<SymbolTable<Value>>(SymbolTable<Value>());
        new_environment->set_enclosing(std::move(this->environment));
        this->environment = std::move(new_environment);

        for (auto &stmt : block->stmts)
        {
            stmt->accept(this);
        }

        this->environment = this->environment->get_enclosing();
    }

    void visit_decl_stmt(DeclStmt *decl_stmt)
    {
        decl_stmt->value->accept(this);

        auto result = this->stack.back();
        this->stack.pop_back();

        this->environment->insert(decl_stmt->identifier.lexeme, result);
    }

    void visit_expr_stmt(ExprStmt *expr_stmt)
    {
        expr_stmt->expr->accept(this);
    }

    void visit_print_stmt(PrintStmt *print_stmt)
    {
        for (auto &arg : print_stmt->args)
        {
            arg->accept(this);
            auto result = this->stack[this->stack.size() - 1];
            this->stack.pop_back();

            std::cout << std::get<int>(result.data);
        }
        std::cout << std::endl;
    }

    void visit_const_stmt(ConstStmt *const_stmt)
    {
        throw BirdException("implement const statment visit");
    }

    void visit_while_stmt(WhileStmt *while_stmt)
    {
        throw BirdException("Implement while statement interpreter");
    }

    void visit_binary(Binary *binary)
    {
        binary->left->accept(this);
        binary->right->accept(this);

        auto right = this->stack.back();
        this->stack.pop_back();

        auto left = this->stack.back();
        this->stack.pop_back();

        switch (binary->op.token_type)
        {
        case Token::Type::PLUS:
        {
            this->stack.push_back(Value(DataType::INT, variant(std::get<int>(left.data) + std::get<int>(right.data))));
            break;
        }
        case Token::Type::MINUS:
        {
            this->stack.push_back(Value(DataType::INT, variant(std::get<int>(left.data) - std::get<int>(right.data))));
            break;
        }
        case Token::Type::SLASH:
        {
            this->stack.push_back(Value(DataType::INT, variant(std::get<int>(left.data) / std::get<int>(right.data))));
            break;
        }
        case Token::Type::STAR:
        {
            this->stack.push_back(Value(DataType::INT, variant(std::get<int>(left.data) * std::get<int>(right.data))));
            break;
        }
        case Token::Type::GREATER:
        {
            this->stack.push_back(Value(DataType::INT, variant(std::get<int>(left.data) > std::get<int>(right.data))));
            break;
        }
        case Token::Type::GREATER_EQUAL:
        {
            this->stack.push_back(Value(DataType::INT, variant(std::get<int>(left.data) >= std::get<int>(right.data))));
            break;
        }
        case Token::Type::LESS:
        {
            this->stack.push_back(Value(DataType::INT, variant(std::get<int>(left.data) < std::get<int>(right.data))));
            break;
        }
        case Token::Type::LESS_EQUAL:
        {
            this->stack.push_back(Value(DataType::INT, variant(std::get<int>(left.data) <= std::get<int>(right.data))));
            break;
        }
        case Token::Type::BANG_EQUAL:
        {
            this->stack.push_back(Value(DataType::INT, variant(std::get<int>(left.data) != std::get<int>(right.data))));
            break;
        }
        case Token::Type::EQUAL_EQUAL:
        {
            this->stack.push_back(Value(DataType::INT, variant(std::get<int>(left.data) == std::get<int>(right.data))));
            break;
        }
        default:
        {
            throw BirdException("undefined binary operator");
        }
        }
    }

    void visit_unary(Unary *unary)
    {
        unary->expr->accept(this);
        auto expr = this->stack[this->stack.size() - 1];
        this->stack.pop_back();

        this->stack.push_back(Value(DataType::INT, variant(-std::get<int>(expr.data))));
    }

    void visit_primary(Primary *primary)
    {
        switch (primary->value.token_type)
        {
        case Token::Type::FLOAT_LITERAL:
        {
            this->stack.push_back(Value(DataType::FLOAT,
                                        variant(std::stof(primary->value.lexeme))));
            break;
        }
        case Token::Type::BOOL_LITERAL:
        {
            this->stack.push_back(Value(DataType::BOOL,
                                        variant(primary->value.lexeme == "true" ? true : false)));
            break;
        }
        case Token::Type::STR_LITERAL:
        {
            this->stack.push_back(Value(DataType::STR,
                                        variant(primary->value.lexeme)));
            break;
        }
        case Token::Type::INT_LITERAL:
        {
            this->stack.push_back(Value(DataType::INT,
                                        variant(std::stoi(primary->value.lexeme))));
            break;
        }
        case Token::Type::IDENTIFIER:
        {
            auto value = this->environment->get(primary->value.lexeme);
            this->stack.push_back(value);
            break;
        }
        default:
        {
            throw BirdException("undefined primary value");
        }
        }
    }

    void visit_if_stmt(IfStmt *if_stmt)
    {
        throw BirdException("implement if statement visit");
    }
};
