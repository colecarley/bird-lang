#pragma once

#include <memory>
#include <vector>
#include <map>
#include "../ast_node/stmt/stmt.h"
#include "../ast_node/expr/expr.h"

#include "../ast_node/expr/binary.h"
#include "../ast_node/expr/unary.h"
#include "../ast_node/expr/primary.h"
#include "../ast_node/expr/ternary.h"
#include "../ast_node/expr/call.h"

#include "../ast_node/stmt/decl_stmt.h"
#include "../ast_node/stmt/assign_stmt.h"
#include "../ast_node/stmt/expr_stmt.h"
#include "../ast_node/stmt/print_stmt.h"
#include "../ast_node/stmt/while_stmt.h"
#include "../ast_node/stmt/if_stmt.h"
#include "../ast_node/stmt/block.h"

#include "../exceptions/bird_exception.h"
#include "../sym_table.h"

#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/TargetParser/Host.h"

/*
 * Visitor that generates the LLVM IR representation of the code
 */
class CodeGen : public Visitor
{
    std::vector<llvm::Value *> stack;
    std::unique_ptr<SymbolTable<llvm::Value *>> environment;
    std::map<std::string, llvm::FunctionCallee> std_lib;

    llvm::LLVMContext context;
    llvm::IRBuilder<> builder;

public:
    std::unique_ptr<llvm::Module> module;

    ~CodeGen()
    {
        for (auto val : this->stack)
        {
            free(val);
        }
    }

    CodeGen() : builder(llvm::IRBuilder<>(this->context))
    {
        this->environment = std::make_unique<SymbolTable<llvm::Value *>>(SymbolTable<llvm::Value *>());
        this->module = std::make_unique<llvm::Module>("test_module", this->context);
    }

    void init_std_lib()
    {
        // Declare the printf function (external function)
        // How can we do this better?
        llvm::FunctionType *printfType = llvm::FunctionType::get(this->builder.getInt32Ty(), this->builder.getPtrTy(), true);
        llvm::FunctionCallee printfFunc = this->module->getOrInsertFunction("printf", printfType);
        this->std_lib["print"] = printfFunc;
        // TODO: when to free printf?
    }

    llvm::BasicBlock *create_entry_point()
    {
        // main function
        llvm::FunctionType *funcType = llvm::FunctionType::get(this->builder.getVoidTy(), false);
        llvm::Function *mainFunction = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, "main", this->module.get());
        llvm::BasicBlock *entry = llvm::BasicBlock::Create(this->context, "entry", mainFunction);

        // TODO: when to free entry block?
        return entry;
    }

    void generate(std::vector<std::unique_ptr<Stmt>> *stmts)
    {
        this->init_std_lib();
        auto entry = this->create_entry_point();

        this->builder.SetInsertPoint(entry);

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

            if (auto if_stmt = dynamic_cast<IfStmt *>(stmt.get()))
            {
                if_stmt->accept(this);
            }

            if (auto block = dynamic_cast<Block *>(stmt.get()))
            {
                block->accept(this);
            }

            if (auto expr_stmt = dynamic_cast<ExprStmt *>(stmt.get()))
            {
                expr_stmt->accept(this);
            }

            if (auto ternary_stmt = dynamic_cast<Ternary *>(stmt.get()))
            {
                ternary_stmt->accept(this);
            }

            if (auto while_stmt = dynamic_cast<WhileStmt *>(stmt.get()))
            {
                while_stmt->accept(this);
            }
        }

        this->builder.CreateRetVoid();

        this->module->print(llvm::outs(), nullptr);
        this->stack.clear();

        std::error_code ll_EC;
        llvm::raw_fd_ostream ll_dest("output.ll", ll_EC);

        if (ll_EC)
        {
            llvm::errs() << "Could not open file: " << ll_EC.message();
            exit(1);
        }

        this->module->print(ll_dest, nullptr);
        ll_dest.close();

        auto TargetTriple = llvm::Triple("wasm32-unknown-unknown");

        llvm::InitializeAllTargetInfos();
        llvm::InitializeAllTargets();
        llvm::InitializeAllTargetMCs();
        llvm::InitializeAllAsmParsers();
        llvm::InitializeAllAsmPrinters();

        std::string Error;
        auto Target = llvm::TargetRegistry::lookupTarget(TargetTriple.getTriple(), Error);

        if (!Target)
        {
            llvm::errs() << Error;
            return;
        }

        auto CPU = "generic";
        auto Features = "";

        llvm::TargetOptions opt;
        auto TargetMachine = Target->createTargetMachine(TargetTriple.getTriple(), CPU, Features, opt, llvm::Reloc::PIC_);

        this->module->setDataLayout(TargetMachine->createDataLayout());
        this->module->setTargetTriple(TargetTriple.getTriple());

        auto Filename = "output.o";
        std::error_code EC;
        llvm::raw_fd_ostream dest(Filename, EC, llvm::sys::fs::OF_None);

        if (EC)
        {
            llvm::errs() << "Could not open file: " << EC.message();
            return;
        }

        llvm::legacy::PassManager pass;
        auto FileType = llvm::CodeGenFileType::ObjectFile;

        if (TargetMachine->addPassesToEmitFile(pass, dest, nullptr, FileType))
        {
            llvm::errs() << "TargetMachine can't emit a file of this type";
            return;
        }

        pass.run(*this->module);
        dest.flush();
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

    void visit_assign_stmt(AssignStmt *assign_stmt)
    {
        throw BirdException("Implement assign statement code gen.");
    }

    void visit_print_stmt(PrintStmt *print_stmt)
    {
        for (auto &arg : print_stmt->args)
        {
            arg->accept(this);
        }

        // TODO: figure out a better way to do this
        auto printfFunc = this->std_lib["print"];

        for (int i = 0; i < print_stmt->args.size(); i++)
        {
            auto result = this->stack.back();
            llvm::Value *formatStr;
            if (result->getType()->isIntegerTy())
            {
                formatStr = this->builder.CreateGlobalString("%d\n");
            }
            else if (result->getType()->isFloatTy())
            {
                formatStr = this->builder.CreateGlobalString("%f\n");
                result = builder.CreateFPExt(result, llvm::Type::getDoubleTy(context));
            }
            else if (result->getType()->isPointerTy())
            {
                formatStr = this->builder.CreateGlobalString("%s\n");
            }
            this->stack.pop_back();
            this->builder.CreateCall(printfFunc, {formatStr, result});
        }
    }

    void visit_expr_stmt(ExprStmt *expr_stmt)
    {
        expr_stmt->expr->accept(this);
    }

    void visit_while_stmt(WhileStmt *while_stmt)
    {
        throw BirdException("Implement while statement code gen");
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
        case Token::Type::PLUS:
        {
            auto value = this->builder.CreateAdd(left, right, "addtmp");
            this->stack.push_back(value);
            break;
        }
        case Token::Type::MINUS:
        {
            auto value = this->builder.CreateSub(left, right, "subtmp");
            this->stack.push_back(value);
            break;
        }
        case Token::Type::SLASH:
        {
            auto value = this->builder.CreateSDiv(left, right, "sdivtmp");
            this->stack.push_back(value);
            break;
        }
        case Token::Type::STAR:
        {
            auto value = this->builder.CreateMul(left, right, "multmp");
            this->stack.push_back(value);
            break;
        }
        /*
         * TODO: comparison operators currently only compare integers, needs type checking
         * 2 < 2.3 would cause it to fail.
         */
        case Token::Type::GREATER:
        {
            auto value = this->builder.CreateICmpSGT(left, right, "sgttmp");
            this->stack.push_back(value);
            break;
        }
        case Token::Type::GREATER_EQUAL:
        {
            auto value = this->builder.CreateICmpSGE(left, right, "sgetmp");
            this->stack.push_back(value);
            break;
        }
        case Token::Type::LESS:
        {
            auto value = this->builder.CreateICmpSLT(left, right, "slttmp");
            this->stack.push_back(value);
            break;
        }
        case Token::Type::LESS_EQUAL:
        {
            auto value = this->builder.CreateICmpSLE(left, right, "sletmp");
            this->stack.push_back(value);
            break;
        }
        case Token::Type::EQUAL_EQUAL:
        {
            auto value = this->builder.CreateICmpEQ(left, right, "eqtmp");
            this->stack.push_back(value);
            break;
        }
        case Token::Type::BANG_EQUAL:
        {
            auto value = this->builder.CreateICmpNE(left, right, "netmp");
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

        auto llvm_value = this->builder.CreateNeg(expr);
        this->stack.push_back(
            llvm_value);
    }

    void visit_primary(Primary *primary)
    {
        switch (primary->value.token_type)
        {
        case Token::Type::INT_LITERAL:
        {
            int value = std::stoi(primary->value.lexeme);

            auto llvm_value = llvm::ConstantInt::get(this->context, llvm::APInt(32, value));
            this->stack.push_back(llvm_value);
            break;
        }
        case Token::Type::FLOAT_LITERAL:
        {
            float value = std::stof(primary->value.lexeme);

            auto llvm_value = llvm::ConstantFP::get(this->context, llvm::APFloat(value));
            this->stack.push_back(llvm_value);
            break;
        }
        case Token::Type::BOOL_LITERAL:
            this->stack.push_back(
                primary->value.lexeme == "true" ? this->builder.getTrue() : this->builder.getFalse());
            break;
        case Token::Type::STR_LITERAL:
        {
            // TODO: figure out strings
            auto value = primary->value.lexeme;
            auto llvm_value = this->builder.CreateGlobalString(value);

            this->stack.push_back(llvm_value);
            break;
        }
        case Token::Type::IDENTIFIER:
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

    void visit_ternary(Ternary *ternary)
    {
        ternary->condition->accept(this);
        // condition contains LLVM IR i1 bool
        auto condition = this->stack.back();
        this->stack.pop_back();

        // control flow requires parent
        auto parent_fn = this->builder.GetInsertBlock()->getParent();

        // create blocks for true and false expressions in the ternary operation, and a done block to merge control flow
        auto true_block = llvm::BasicBlock::Create(this->context, "true", parent_fn);
        auto flase_block = llvm::BasicBlock::Create(this->context, "false", parent_fn);
        auto done_block = llvm::BasicBlock::Create(this->context, "done", parent_fn);

        // create conditional branch for condition, if true -> true block, if false -> false block
        this->builder.CreateCondBr(condition, true_block, flase_block);

        this->builder.SetInsertPoint(true_block); // set insert point to true block for true expression instuctions
        ternary->true_expr->accept(this);
        auto true_result = this->stack.back();
        this->stack.pop_back();

        this->builder.CreateBr(done_block); // create branch to merge control flow

        // update true block for phi node reference to predecessor
        true_block = this->builder.GetInsertBlock();

        this->builder.SetInsertPoint(flase_block); // set insert point for false block for false expression instructions
        ternary->false_expr->accept(this);
        auto false_result = this->stack.back();
        this->stack.pop_back();

        this->builder.CreateBr(done_block);

        // update false block for phi node reference to predecessor
        flase_block = this->builder.GetInsertBlock();

        // set insert point for done (merge) block
        this->builder.SetInsertPoint(done_block);

        /*
         * phi node which has type `result`, 2 incoming values, and is named "ternary result"
         * this is how we select the correct result based on which branch is executed
         */
        auto phi_node = this->builder.CreatePHI(true_result->getType(), 2, "ternary result");
        phi_node->addIncoming(true_result, true_block);   // associates true result to true block
        phi_node->addIncoming(false_result, flase_block); // associates false result to false block

        this->stack.push_back(phi_node); // push the phi node w branch result on to the stack
    }

    void visit_const_stmt(ConstStmt *const_stmt)
    {
        throw BirdException("implement const statment visit");
    }

    void visit_func(Func *func)
    {
        throw BirdException("implement func visit");
    }

    void visit_if_stmt(IfStmt *if_stmt)
    {
        if_stmt->condition->accept(this);

        auto condition = this->stack.back();
        this->stack.pop_back();

        auto parent_fn = this->builder.GetInsertBlock()->getParent();
        if (if_stmt->else_branch.has_value())
        {
            auto then_block = llvm::BasicBlock::Create(this->context, "then", parent_fn);
            auto else_block = llvm::BasicBlock::Create(this->context, "else", parent_fn);
            auto done_block = llvm::BasicBlock::Create(this->context, "if_cont", parent_fn);

            this->builder.CreateCondBr(condition, then_block, else_block);

            this->builder.SetInsertPoint(then_block);
            if_stmt->then_branch->accept(this);
            this->builder.CreateBr(done_block);

            this->builder.SetInsertPoint(else_block);
            if_stmt->else_branch.value().get()->accept(this);

            this->builder.CreateBr(done_block);
            this->builder.SetInsertPoint(done_block);
        }
        else
        {
            auto then_block = llvm::BasicBlock::Create(this->context, "then", parent_fn);
            auto done_block = llvm::BasicBlock::Create(this->context, "if_cont", parent_fn);

            this->builder.CreateCondBr(condition, then_block, done_block);

            this->builder.SetInsertPoint(then_block);
            if_stmt->then_branch->accept(this);

            this->builder.CreateBr(done_block);
            this->builder.SetInsertPoint(done_block);
        }
    }

    void visit_call(Call *call)
    {
        throw BirdException("implement call");
    }
};
