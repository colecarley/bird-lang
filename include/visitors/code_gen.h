#pragma once

#include <memory>
#include <vector>
#include <map>
#include "../ast_node/stmt/stmt.h"
#include "../ast_node/expr/expr.h"

#include "../ast_node/expr/binary.h"
#include "../ast_node/expr/unary.h"
#include "../ast_node/expr/primary.h"

#include "../ast_node/stmt/decl_stmt.h"
#include "../ast_node/stmt/expr_stmt.h"
#include "../ast_node/stmt/print_stmt.h"
#include "../ast_node/stmt/block.h"

#include "../exceptions/bird_exception.h"
#include "../sym_table.h"

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Verifier.h>

// TODO: figure out a way to put this in the class
static llvm::LLVMContext TheContext;
static llvm::IRBuilder<> Builder(TheContext);

/*
 * Visitor that generates the LLVM IR representation of the code
 */
class CodeGen : public Visitor
{
    std::vector<llvm::Value *> stack;
    std::unique_ptr<SymbolTable<llvm::Value *>> environment;
    std::map<std::string, llvm::FunctionCallee> std_lib;

public:
    CodeGen()
    {
        this->environment = std::make_unique<SymbolTable<llvm::Value *>>(SymbolTable<llvm::Value *>());
    }

    void init_std_lib(llvm::Module *TheModule)
    {
        // Declare the printf function (external function)
        llvm::FunctionType *printfType = llvm::FunctionType::get(Builder.getInt32Ty(), Builder.getPtrTy(), true);
        llvm::FunctionCallee printfFunc = TheModule->getOrInsertFunction("printf", printfType);
        this->std_lib["print"] = printfFunc;
        // TODO: when to free printf?
    }

    llvm::BasicBlock *create_entry_point(llvm::Module *TheModule)
    {
        // main function
        llvm::FunctionType *funcType = llvm::FunctionType::get(Builder.getVoidTy(), false);
        llvm::Function *mainFunction = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, "main", TheModule);
        llvm::BasicBlock *entry = llvm::BasicBlock::Create(TheContext, "entry", mainFunction);

        // TODO: when to free entry block?
        return entry;
    }

    llvm::Module *generate(std::vector<std::unique_ptr<Stmt>> *stmts)
    {
        llvm::Module *TheModule = new llvm::Module("my_module", TheContext);

        this->init_std_lib(TheModule);
        auto entry = this->create_entry_point(TheModule);

        Builder.SetInsertPoint(entry);

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
        }

        Builder.CreateRetVoid();

        TheModule->print(llvm::outs(), nullptr);
        this->stack.clear();

        std::error_code EC;
        llvm::raw_fd_ostream dest("output.ll", EC);

        if (EC)
        {
            llvm::errs() << "Could not open file: " << EC.message();
            exit(1);
        }

        TheModule->print(dest, nullptr);
        dest.close();

        return TheModule;
    }

    void visit_block(Block *block)
    {
        auto new_environment = std::make_unique<SymbolTable<llvm::Value *>>(SymbolTable<llvm::Value *>());
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

        llvm::Value *result = this->stack[this->stack.size() - 1];
        this->stack.pop_back();

        this->environment->insert(decl_stmt->identifier.lexeme, result);
    }

    void visit_print_stmt(PrintStmt *print_stmt)
    {
        for (auto &arg : print_stmt->args)
        {
            arg->accept(this);
        }

        auto printfFunc = this->std_lib["print"];
        llvm::Value *formatStr = Builder.CreateGlobalStringPtr("%d");
        for (int i = 0; i < print_stmt->args.size(); i++)
        {
            auto result = this->stack[this->stack.size() - 1];
            this->stack.pop_back();
            Builder.CreateCall(printfFunc, {formatStr, result});
        }

        llvm::Value *newline = Builder.CreateGlobalStringPtr("\n");
        Builder.CreateCall(printfFunc, newline);
    }

    void visit_expr_stmt(ExprStmt *expr_stmt)
    {
        expr_stmt->expr->accept(this);
    }

    void visit_binary(Binary *binary)
    {
        binary->left->accept(this);
        binary->right->accept(this);

        auto right = this->stack[this->stack.size() - 1];
        this->stack.pop_back();

        auto left = this->stack[this->stack.size() - 1];
        this->stack.pop_back();

        switch (binary->op.token_type)
        {
        case TokenType::PLUS:
        {
            auto value = Builder.CreateAdd(left, right, "addtmp");
            this->stack.push_back(value);
            break;
        }
        case TokenType::MINUS:
        {
            auto value = Builder.CreateSub(left, right, "subtmp");
            this->stack.push_back(value);
            break;
        }
        case TokenType::SLASH:
        {
            auto value = Builder.CreateSDiv(left, right, "sdivtmp");
            this->stack.push_back(value);
            break;
        }
        case TokenType::STAR:
        {
            auto value = Builder.CreateMul(left, right, "multmp");
            this->stack.push_back(value);
            break;
        }
        default:
        {
            throw BirdException("undefined binary operator for code gen");
        }
        }
    }

    void visit_unary(Unary *unary)
    {
        unary->expr->accept(this);
        auto expr = this->stack[this->stack.size() - 1];
        this->stack.pop_back();

        auto llvm_value = Builder.CreateNeg(expr);
        this->stack.push_back(
            llvm_value);
    }

    void visit_primary(Primary *primary)
    {
        switch (primary->value.token_type)
        {
        case TokenType::I32_LITERAL:
        {
            float value = std::stoi(primary->value.lexeme);

            auto llvm_value = llvm::ConstantInt::get(TheContext, llvm::APInt(32, value));
            this->stack.push_back(llvm_value);
            break;
        }
        case TokenType::IDENTIFIER:
        {
            auto value = this->environment->get(primary->value.lexeme);
            if (value == nullptr)
            {
                throw BirdException("undefined identifier");
            }
            this->stack.push_back(value);
            break;
        }
        default:
        {
            throw BirdException("undefined primary value");
        }
        }
    }
};
