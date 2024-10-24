#pragma once

#include <memory>
#include <vector>
#include <map>
#include <functional>
#include <stack>
#include <algorithm>

#include "../ast_node/stmt/stmt.h"
#include "../ast_node/expr/expr.h"

#include "../ast_node/expr/binary.h"
#include "../ast_node/expr/unary.h"
#include "../ast_node/expr/primary.h"
#include "../ast_node/expr/ternary.h"
#include "../ast_node/expr/call.h"

#include "../ast_node/stmt/decl_stmt.h"
#include "../ast_node/expr/assign_expr.h"
#include "../ast_node/stmt/expr_stmt.h"
#include "../ast_node/stmt/print_stmt.h"
#include "../ast_node/stmt/const_stmt.h"
#include "../ast_node/stmt/while_stmt.h"
#include "../ast_node/stmt/for_stmt.h"
#include "../ast_node/stmt/return_stmt.h"
#include "../ast_node/stmt/if_stmt.h"
#include "../ast_node/stmt/block.h"
#include "../ast_node/stmt/func.h"
#include "../ast_node/stmt/break_stmt.h"
#include "../ast_node/stmt/continue_stmt.h"

#include "../sym_table.h"
#include "../exceptions/bird_exception.h"
#include "../exceptions/return_exception.h"
#include "../bird_type.h"

/*
 * Visitor that checks types of the AST
 */
class TypeChecker : public Visitor
{

public:
    std::shared_ptr<SymbolTable<BirdType>> environment;
    std::shared_ptr<SymbolTable<BirdFunction>> call_table;
    std::stack<BirdType> stack;

    TypeChecker()
    {
        this->environment = std::make_shared<SymbolTable<BirdType>>();
        this->call_table = std::make_shared<SymbolTable<BirdFunction>>();
    }

    std::map<Token::Type, std::map<std::pair<BirdType, BirdType>, std::function<BirdType()>>> binary_operations =
        {
            {Token::Type::PLUS,
             {
                 {{BirdType::INT, BirdType::INT}, []()
                  { return BirdType::INT; }},
                 {{BirdType::FLOAT, BirdType::FLOAT}, []()
                  { return BirdType::FLOAT; }},
                 {{BirdType::INT, BirdType::FLOAT}, []()
                  { return BirdType::INT; }},
                 {{BirdType::FLOAT, BirdType::INT}, []()
                  { return BirdType::FLOAT; }},
                 {{BirdType::STRING, BirdType::STRING}, []()
                  { return BirdType::STRING; }},
             }},
            {Token::Type::MINUS,
             {
                 {{BirdType::INT, BirdType::INT}, []()
                  { return BirdType::INT; }},
                 {{BirdType::FLOAT, BirdType::FLOAT}, []()
                  { return BirdType::FLOAT; }},
                 {{BirdType::INT, BirdType::FLOAT}, []()
                  { return BirdType::INT; }},
                 {{BirdType::FLOAT, BirdType::INT}, []()
                  { return BirdType::FLOAT; }},
             }},
            {Token::Type::STAR,
             {
                 {{BirdType::INT, BirdType::INT}, []()
                  { return BirdType::INT; }},
                 {{BirdType::FLOAT, BirdType::FLOAT}, []()
                  { return BirdType::FLOAT; }},
                 {{BirdType::INT, BirdType::FLOAT}, []()
                  { return BirdType::INT; }},
                 {{BirdType::FLOAT, BirdType::INT}, []()
                  { return BirdType::FLOAT; }},
             }},
            {Token::Type::SLASH,
             {
                 {{BirdType::INT, BirdType::INT}, []()
                  { return BirdType::INT; }},
                 {{BirdType::FLOAT, BirdType::FLOAT}, []()
                  { return BirdType::FLOAT; }},
                 {{BirdType::INT, BirdType::FLOAT}, []()
                  { return BirdType::INT; }},
                 {{BirdType::FLOAT, BirdType::INT}, []()
                  { return BirdType::FLOAT; }},
             }},
            {Token::Type::EQUAL_EQUAL,
             {
                 {{BirdType::INT, BirdType::INT}, []()
                  { return BirdType::BOOL; }},
                 {{BirdType::FLOAT, BirdType::FLOAT}, []()
                  { return BirdType::BOOL; }},
                 {{BirdType::INT, BirdType::FLOAT}, []()
                  { return BirdType::BOOL; }},
                 {{BirdType::FLOAT, BirdType::INT}, []()
                  { return BirdType::BOOL; }},
                 {{BirdType::BOOL, BirdType::BOOL}, []()
                  { return BirdType::BOOL; }},
                 {{BirdType::STRING, BirdType::STRING}, []()
                  { return BirdType::BOOL; }},
             }},
            {Token::Type::BANG_EQUAL,
             {
                 {{BirdType::INT, BirdType::INT}, []()
                  { return BirdType::BOOL; }},
                 {{BirdType::FLOAT, BirdType::FLOAT}, []()
                  { return BirdType::BOOL; }},
                 {{BirdType::INT, BirdType::FLOAT}, []()
                  { return BirdType::BOOL; }},
                 {{BirdType::FLOAT, BirdType::INT}, []()
                  { return BirdType::BOOL; }},
                 {{BirdType::BOOL, BirdType::BOOL}, []()
                  { return BirdType::BOOL; }},
                 {{BirdType::STRING, BirdType::STRING}, []()
                  { return BirdType::BOOL; }},
             }},
            {Token::Type::LESS,
             {
                 {{BirdType::INT, BirdType::INT}, []()
                  { return BirdType::BOOL; }},
                 {{BirdType::FLOAT, BirdType::FLOAT}, []()
                  { return BirdType::BOOL; }},
                 {{BirdType::INT, BirdType::FLOAT}, []()
                  { return BirdType::BOOL; }},
                 {{BirdType::FLOAT, BirdType::INT}, []()
                  { return BirdType::BOOL; }},
                 {{BirdType::STRING, BirdType::STRING}, []()
                  { return BirdType::BOOL; }},
             }},
            {Token::Type::LESS_EQUAL,
             {
                 {{BirdType::INT, BirdType::INT}, []()
                  { return BirdType::BOOL; }},
                 {{BirdType::FLOAT, BirdType::FLOAT}, []()
                  { return BirdType::BOOL; }},
                 {{BirdType::INT, BirdType::FLOAT}, []()
                  { return BirdType::BOOL; }},
                 {{BirdType::FLOAT, BirdType::INT}, []()
                  { return BirdType::BOOL; }},
                 {{BirdType::STRING, BirdType::STRING}, []()
                  { return BirdType::BOOL; }},
             }},
            {Token::Type::GREATER,
             {
                 {{BirdType::INT, BirdType::INT}, []()
                  { return BirdType::BOOL; }},
                 {{BirdType::FLOAT, BirdType::FLOAT}, []()
                  { return BirdType::BOOL; }},
                 {{BirdType::INT, BirdType::FLOAT}, []()
                  { return BirdType::BOOL; }},
                 {{BirdType::FLOAT, BirdType::INT}, []()
                  { return BirdType::BOOL; }},
                 {{BirdType::STRING, BirdType::STRING}, []()
                  { return BirdType::BOOL; }},
             }},
            {Token::Type::GREATER_EQUAL,
             {
                 {{BirdType::INT, BirdType::INT}, []()
                  { return BirdType::BOOL; }},
                 {{BirdType::FLOAT, BirdType::FLOAT}, []()
                  { return BirdType::BOOL; }},
                 {{BirdType::INT, BirdType::FLOAT}, []()
                  { return BirdType::BOOL; }},
                 {{BirdType::FLOAT, BirdType::INT}, []()
                  { return BirdType::BOOL; }},
                 {{BirdType::STRING, BirdType::STRING}, []()
                  { return BirdType::BOOL; }},
             }},
            {Token::Type::PERCENT,
             {
                 {{BirdType::INT, BirdType::INT}, []()
                  { return BirdType::INT; }},
                 {{BirdType::FLOAT, BirdType::FLOAT}, []()
                  { return BirdType::FLOAT; }},
                 {{BirdType::INT, BirdType::FLOAT}, []()
                  { return BirdType::INT; }},
                 {{BirdType::FLOAT, BirdType::INT}, []()
                  { return BirdType::FLOAT; }},
             }},
        };

    void
    check_types(std::vector<std::unique_ptr<Stmt>> *stmts)
    {
        for (auto &stmt : *stmts)
        {
            if (auto decl_stmt = dynamic_cast<DeclStmt *>(stmt.get()))
            {
                decl_stmt->accept(this);
                continue;
            }

            if (auto const_stmt = dynamic_cast<ConstStmt *>(stmt.get()))
            {
                const_stmt->accept(this);
                continue;
            }

            if (auto assign_expr = dynamic_cast<AssignExpr *>(stmt.get()))
            {
                assign_expr->accept(this);
                continue;
            }

            if (auto print_stmt = dynamic_cast<PrintStmt *>(stmt.get()))
            {
                print_stmt->accept(this);
                continue;
            }

            if (auto block = dynamic_cast<Block *>(stmt.get()))
            {
                block->accept(this);
                continue;
            }

            if (auto expr_stmt = dynamic_cast<ExprStmt *>(stmt.get()))
            {
                expr_stmt->accept(this);
                continue;
            }

            if (auto while_stmt = dynamic_cast<WhileStmt *>(stmt.get()))
            {
                while_stmt->accept(this);
                continue;
            }

            if (auto for_stmt = dynamic_cast<ForStmt *>(stmt.get()))
            {
                for_stmt->accept(this);
                continue;
            }

            if (auto if_stmt = dynamic_cast<IfStmt *>(stmt.get()))
            {
                if_stmt->accept(this);
                continue;
            }

            if (auto func = dynamic_cast<Func *>(stmt.get()))
            {
                func->accept(this);
                continue;
            }

            if (auto return_stmt = dynamic_cast<ReturnStmt *>(stmt.get()))
            {
                return_stmt->accept(this);
            }

            if (auto break_stmt = dynamic_cast<BreakStmt *>(stmt.get()))
            {
                break_stmt->accept(this);
                continue;
            }

            if (auto continue_stmt = dynamic_cast<ContinueStmt *>(stmt.get()))
            {
                continue_stmt->accept(this);
                continue;
            }
        }

        while (!this->stack.empty())
        {
            this->stack.pop();
        }
    }

    void visit_block(Block *block)
    {
        std::shared_ptr<SymbolTable<BirdType>> new_environment =
            std::make_shared<SymbolTable<BirdType>>();

        for (auto &stmt : block->stmts)
        {
            stmt->accept(this);
        }

        this->environment = new_environment->get_enclosing();
    }

    void visit_decl_stmt(DeclStmt *decl_stmt)
    {
        // TODO: implement
    }

    void visit_assign_expr(AssignExpr *assign_expr)
    {
        // TODO: implement
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
        }
    }

    void visit_const_stmt(ConstStmt *const_stmt)
    {
    }

    void visit_while_stmt(WhileStmt *while_stmt)
    {
        auto previous_environment = this->environment; // TODO: pull this out into its own function

        while_stmt->condition->accept(this);
        auto condition_result = std::move(this->stack.top());
        this->stack.pop();

        if (condition_result != BirdType::BOOL)
            throw BirdException("expected bool in while statement condition");

        try
        {
            while_stmt->stmt->accept(this);
        }
        catch (BreakException e)
        {
            // do nothing
        }
        catch (ContinueException e)
        {
            while_stmt->condition->accept(this);
            condition_result = std::move(this->stack.top());
            this->stack.pop();

            this->environment = previous_environment;
        }
    }

    void visit_for_stmt(ForStmt *for_stmt)
    {
        std::shared_ptr<SymbolTable<BirdType>> new_environment =
            std::make_shared<SymbolTable<BirdType>>();
        new_environment->set_enclosing(this->environment);
        this->environment = new_environment;

        if (for_stmt->initializer.has_value())
        {
            for_stmt->initializer.value()->accept(this);
        }

        if (for_stmt->condition.has_value())
        {
            for_stmt->condition.value()->accept(this);
            auto condition_result = std::move(this->stack.top());
            this->stack.pop();

            if (condition_result != BirdType::BOOL)
            {
                throw BirdException("expected bool in for statement condition");
            }
        }

        if (for_stmt->increment.has_value())
        {
            for_stmt->increment.value()->accept(this);
        }

        this->environment = this->environment->get_enclosing();
    }

    void visit_binary(Binary *binary)
    {
        binary->left->accept(this);
        binary->right->accept(this);

        auto right = std::move(this->stack.top());
        this->stack.pop();

        // TODO: investigate these moves
        auto left = std::move(this->stack.top());
        this->stack.pop();

        auto operator_options = this->binary_operations[binary->op.token_type];
        if (operator_options.find({left, right}) == operator_options.end())
        {
            throw BirdException("unsupported types for binary operation"); // TODO: make type error
        }

        auto function = operator_options[{left, right}];
        this->stack.push(function());
    }

    void visit_unary(Unary *unary)
    {
        unary->expr->accept(this);
        auto result = std::move(this->stack.top());

        if (result == BirdType::INT)
        {
            this->stack.push(BirdType::INT);
        }
        else if (result == BirdType::FLOAT)
        {
            this->stack.push(BirdType::FLOAT);
        }
        else
        {
            throw BirdException("unsupported types for unary operation");
        }
    }

    void visit_primary(Primary *primary)
    {
        switch (primary->value.token_type)
        {
        case Token::Type::FLOAT_LITERAL:
        {
            this->stack.push(BirdType::FLOAT);
            break;
        }
        case Token::Type::INT_LITERAL:
        {
            this->stack.push(BirdType::INT);
            std::cout << "found int" << std::endl;
            break;
        }
        case Token::Type::BOOL_LITERAL:
        {
            this->stack.push(BirdType::BOOL);
            break;
        }
        case Token::Type::STR_LITERAL:
        {
            this->stack.push(BirdType::STRING);
            break;
        }
        case Token::Type::IDENTIFIER:
        {
            this->stack.push(
                this->environment->get(primary->value.lexeme));
            break;
        }
        default:
        {
            throw BirdException("undefined primary value");
        }
        }
    }

    void visit_ternary(Ternary *ternary)
    {
        ternary->condition->accept(this);
        auto condition = std::move(this->stack.top());
        this->stack.pop();

        if (condition != BirdType::BOOL)
        {
            throw BirdException("expected bool result for ternary condition");
        }

        ternary->true_expr->accept(this);
        auto true_expr = std::move(this->stack.top());

        ternary->false_expr->accept(this);
        auto false_expr = std::move(this->stack.top());

        if (true_expr != false_expr)
        {
            throw BirdException("mismatching types in ternary expression");
        }

        this->stack.push(true_expr);
    }

    BirdType get_type_from_token(Token token)
    {
        auto type = token.lexeme;
        if (type == "int")
        {
            return BirdType::INT;
        }
        else if (type == "float")
        {
            return BirdType::FLOAT;
        }
        else if (type == "bool")
        {
            return BirdType::BOOL;
        }
        else if (type == "str")
        {
            return BirdType::STRING;
        }
        else if (type == "void")
        {
            return BirdType::VOID;
        }
    }

    void visit_func(Func *func)
    {
        std::vector<BirdType>
            params;
        std::transform(func->param_list.begin(), func->param_list.end(), std::back_inserter(params), [&](auto param)
                       { return this->get_type_from_token(param.second); });

        BirdType ret = func->return_type.has_value()                       ? [&]()
        { return this->get_type_from_token(func->return_type.value()); }() : BirdType::VOID;

        this->call_table->insert(func->identifier.lexeme, BirdFunction(params, ret));

        std::shared_ptr<SymbolTable<BirdType>> new_environment =
            std::make_shared<SymbolTable<BirdType>>();
        new_environment->set_enclosing(this->environment);
        this->environment = new_environment;

        for (auto &param : func->param_list)
        {
            this->environment->insert(param.first.lexeme, this->get_type_from_token(param.second));
        }

        for (auto &stmt : dynamic_cast<Block *>(func->block.get())->stmts)
        {
            try
            {
                stmt->accept(this);
            }
            catch (ReturnException e)
            {
                auto result = std::move(this->stack.top());
                this->stack.pop();

                if (result != ret)
                {
                    std::cout << "result: " << int(result) << std::endl;
                    std::cout << "ret: " << int(ret) << std::endl;
                    throw BirdException("mismatching return type in function");
                }
            }
        }

        this->environment = this->environment->get_enclosing();
    }

    void visit_if_stmt(IfStmt *if_stmt)
    {
        if_stmt->condition->accept(this);
        auto condition = std::move(this->stack.top());
        this->stack.pop();

        if (condition != BirdType::BOOL)
        {
            throw BirdException("expected bool result for if statement condition");
        }

        if_stmt->then_branch->accept(this);

        if (if_stmt->else_branch.has_value())
        {
            if_stmt->else_branch.value()->accept(this);
        }
    }

    void visit_call(Call *call)
    {
        auto function = this->call_table->get(call->identifier.lexeme);

        if (function.params.size() != call->args.size())
        {
            throw BirdException("mismatching number of arguments in function call");
        }

        for (int i = 0; i < function.params.size(); i++)
        {
            call->args[i]->accept(this);
            auto arg = std::move(this->stack.top());
            this->stack.pop();

            if (arg != function.params[i])
            {
                throw BirdException("mismatching types in function call");
            }
        }

        this->stack.push(function.ret);
    }

    void visit_return_stmt(ReturnStmt *return_stmt)
    {
        if (return_stmt->expr.has_value())
        {
            return_stmt->expr.value()->accept(this);
            auto result = std::move(this->stack.top());
            this->stack.pop();

            this->stack.push(result);
        }
        else
        {
            this->stack.push(BirdType::VOID);
        }

        throw ReturnException();
    }

    void visit_break_stmt(BreakStmt *break_stmt)
    {
        // do nothing
    }

    void visit_continue_stmt(ContinueStmt *continue_stmt)
    {
        // do nothing
    }
};
