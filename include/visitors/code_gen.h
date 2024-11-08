#pragma once

#include "binaryen-c.h"

class CodeGen : public Visitor
{
    std::shared_ptr<Environment<BinaryenExpressionRef>> environment;
    std::map<std::string, std::string> std_lib;
    Stack<BinaryenExpressionRef> stack;

    std::vector<BinaryenExpressionRef> fn_body;
    std::vector<BinaryenType> locals;

    BinaryenModuleRef mod;
    int local_index;

public:
    ~CodeGen()
    {
        BinaryenModuleDispose(this->mod);
    }

    CodeGen() : mod(BinaryenModuleCreate()), local_index(0)
    {
        this->environment =
            std::make_shared<Environment<BinaryenExpressionRef>>();
    }

    void init_std_lib()
    {
        // format string and pointer as param for printf
        BinaryenType params =
            BinaryenTypeCreate((BinaryenType[]){BinaryenTypeInt32(), BinaryenTypeInt32()}, 2);

        BinaryenType results = BinaryenTypeNone();

        BinaryenAddFunctionImport(
            this->mod,
            "printf",
            "env",
            "printf",
            params,
            results);

        this->std_lib["print"] = "printf";
    }

    BinaryenFunctionRef create_entry_point()
    {
        BinaryenType params = BinaryenTypeNone();
        BinaryenType results = BinaryenTypeNone();

        BinaryenExpressionRef body =
            BinaryenBlock(
                this->mod,
                nullptr,
                fn_body.data(),
                fn_body.size(),
                BinaryenTypeNone());

        BinaryenFunctionRef mainFunction =
            BinaryenAddFunction(
                this->mod,
                "main",
                params,
                results,
                locals.data(),
                locals.size(),
                body);

        BinaryenAddFunctionExport(
            this->mod,
            "main",
            "main");

        return mainFunction;
    }

    void generate(std::vector<std::unique_ptr<Stmt>> *stmts)
    {
        this->init_std_lib();
        this->environment->push_env();

        for (auto &stmt : *stmts)
        {
            if (auto func_stmt = dynamic_cast<Func *>(stmt.get()))
            {
                func_stmt->accept(this);
            }

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

            if (auto for_stmt = dynamic_cast<ForStmt *>(stmt.get()))
            {
                for_stmt->accept(this);
            }

            if (auto return_stmt = dynamic_cast<ReturnStmt *>(stmt.get()))
            {
                return_stmt->accept(this);
            }

            if (auto break_stmt = dynamic_cast<BreakStmt *>(stmt.get()))
            {
                break_stmt->accept(this);
            }

            if (auto continue_stmt = dynamic_cast<ContinueStmt *>(stmt.get()))
            {
                continue_stmt->accept(this);
            }
        }

        create_entry_point();

        BinaryenModulePrint(this->mod);

        BinaryenModuleAllocateAndWriteResult result =
            BinaryenModuleAllocateAndWrite(this->mod, nullptr);

        if (!result.binary || result.binaryBytes == 0)
        {
            std::cerr << "failed to serialize" << std::endl;
            return;
        }

        std::string filename = "output.wasm";
        std::ofstream file(filename, std::ios::binary);
        if (file.is_open())
        {
            file.write(static_cast<char *>(result.binary), result.binaryBytes);
            file.close();
            std::cout << "wasm module written to " << filename << std::endl;
        }
        else
        {
            std::cerr << "failed to open file" << filename << std::endl;
        }

        free(result.binary);

        this->environment->pop_env();
    }

    int allocate_local()
    {
        int index = this->local_index++;
        return index;
    }

    BinaryenType from_bird_type(Token token)
    {
        if (token.lexeme == "bool")
            return BinaryenTypeInt32(); // no i1?
        else if (token.lexeme == "int")
            return BinaryenTypeInt32();
        else if (token.lexeme == "float")
            return BinaryenTypeFloat64();
        else if (token.lexeme == "void")
            return BinaryenTypeNone();
        else if (token.lexeme == "str")
            return BinaryenTypeInt32(); // ptr
        else
            throw BirdException("invalid type");
    }

    void visit_block(Block *block)
    {
        // throw BirdException("Implement visit_block");
    }

    void visit_decl_stmt(DeclStmt *decl_stmt)
    {
        decl_stmt->value->accept(this);
        BinaryenExpressionRef initializer_value = this->stack.pop();

        BinaryenType type = decl_stmt->type_identifier.has_value()
                                ? from_bird_type(decl_stmt->type_identifier.value())
                                : BinaryenExpressionGetType(initializer_value);

        int index = allocate_local(); // locals defined by index instead of name
        locals.push_back(type);

        BinaryenExpressionRef set_local = BinaryenLocalSet(this->mod, index, initializer_value);

        environment->declare(decl_stmt->identifier.lexeme, BinaryenLocalGet(this->mod, index, type));

        fn_body.push_back(set_local);
    }

    void visit_assign_expr(AssignExpr *assign_expr)
    {
        // throw BirdException("Implement Assign Expression");
    }

    void visit_print_stmt(PrintStmt *print_stmt)
    {
        // throw BirdException("Implement Print Statement");
    }

    void visit_expr_stmt(ExprStmt *expr_stmt)
    {
        // throw BirdException("Implement Expr Statement");
    }

    void visit_while_stmt(WhileStmt *while_stmt)
    {
        // throw BirdException("Implement While Statement");
    }

    void visit_for_stmt(ForStmt *for_stmt)
    {
        // throw BirdException("Implement For Statement");
    }

    void visit_binary(Binary *binary)
    {
        binary->left->accept(this);
        binary->right->accept(this);

        auto right = this->stack.pop();
        auto left = this->stack.pop();

        bool float_flag = (BinaryenExpressionGetType(left) == BinaryenTypeFloat64() ||
                           BinaryenExpressionGetType(right) == BinaryenTypeFloat64());

        if (float_flag && BinaryenExpressionGetType(left) == BinaryenTypeInt32())
        {
            left =
                BinaryenUnary(
                    mod,
                    BinaryenConvertSInt32ToFloat64(),
                    left);
        }
        else if (float_flag && BinaryenExpressionGetType(right) == BinaryenTypeInt32())
        {
            right =
                BinaryenUnary(
                    mod,
                    BinaryenConvertSInt32ToFloat64(),
                    right);
        }

        switch (binary->op.token_type)
        {
        case Token::Type::PLUS:
        {
            (float_flag)
                ? this->stack.push(BinaryenBinary(mod, BinaryenAddFloat64(), left, right))
                : this->stack.push(BinaryenBinary(mod, BinaryenAddInt32(), left, right));

            break;
        }
        case Token::Type::MINUS:
        {
            (float_flag)
                ? this->stack.push(BinaryenBinary(mod, BinaryenSubFloat64(), left, right))
                : this->stack.push(BinaryenBinary(mod, BinaryenSubInt32(), left, right));

            break;
        }
        case Token::Type::SLASH:
        {
            (float_flag)
                ? this->stack.push(BinaryenBinary(mod, BinaryenDivFloat64(), left, right))
                : this->stack.push(BinaryenBinary(mod, BinaryenDivSInt32(), left, right));

            break;
        }
        case Token::Type::STAR:
        {
            (float_flag)
                ? this->stack.push(BinaryenBinary(mod, BinaryenMulFloat64(), left, right))
                : this->stack.push(BinaryenBinary(mod, BinaryenMulInt32(), left, right));

            break;
        }
        case Token::Type::GREATER:
        {
            (float_flag)
                ? this->stack.push(BinaryenBinary(mod, BinaryenGtFloat64(), left, right))
                : this->stack.push(BinaryenBinary(mod, BinaryenGtSInt32(), left, right));

            break;
        }
        case Token::Type::GREATER_EQUAL:
        {
            (float_flag)
                ? this->stack.push(BinaryenBinary(mod, BinaryenGeFloat64(), left, right))
                : this->stack.push(BinaryenBinary(mod, BinaryenGeSInt32(), left, right));

            break;
        }
        case Token::Type::LESS:
        {
            (float_flag)
                ? this->stack.push(BinaryenBinary(mod, BinaryenLtFloat64(), left, right))
                : this->stack.push(BinaryenBinary(mod, BinaryenLtSInt32(), left, right));

            break;
        }
        case Token::Type::LESS_EQUAL:
        {
            (float_flag)
                ? this->stack.push(BinaryenBinary(mod, BinaryenLeFloat64(), left, right))
                : this->stack.push(BinaryenBinary(mod, BinaryenLeSInt32(), left, right));

            break;
        }
        case Token::Type::EQUAL_EQUAL:
        {
            (float_flag)
                ? this->stack.push(BinaryenBinary(mod, BinaryenEqFloat64(), left, right))
                : this->stack.push(BinaryenBinary(mod, BinaryenEqInt32(), left, right));

            break;
        }
        case Token::Type::BANG_EQUAL:
        {
            (float_flag)
                ? this->stack.push(BinaryenBinary(mod, BinaryenNeFloat64(), left, right))
                : this->stack.push(BinaryenBinary(mod, BinaryenNeInt32(), left, right));

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
        // throw BirdException("Implement visit_unary");
    }

    void visit_primary(Primary *primary)
    {
        switch (primary->value.token_type)
        {
        case Token::Type::INT_LITERAL:
        {
            int value = std::stoi(primary->value.lexeme);
            BinaryenExpressionRef int_literal = BinaryenConst(this->mod, BinaryenLiteralInt32(value));
            this->stack.push(int_literal);
            break;
        }

        case Token::Type::FLOAT_LITERAL:
        {
            double value = std::stod(primary->value.lexeme);
            BinaryenExpressionRef float_literal = BinaryenConst(this->mod, BinaryenLiteralFloat64(value));
            this->stack.push(float_literal);
            break;
        }

        case Token::Type::BOOL_LITERAL:
        {
            BinaryenExpressionRef bool_literal = BinaryenConst(
                this->mod,
                primary->value.lexeme == "true"
                    ? BinaryenLiteralInt32(1)
                    : BinaryenLiteralInt32(0));

            this->stack.push(bool_literal);
            break;
        }

        case Token::Type::STR_LITERAL:
        {
            break;
        }

        case Token::Type::IDENTIFIER:
        {
            break;
        }

        default:
            throw BirdException("undefined primary value: " + primary->value.lexeme);
        }
    }

    void visit_ternary(Ternary *ternary)
    {
        // throw BirdException("Implement Ternart Statement");
    }

    void visit_const_stmt(ConstStmt *const_stmt)
    {
        // throw BirdException("Implement Const Statement");
    }

    void visit_func(Func *func)
    {
        // throw BirdException("Implement visit_func");
    }

    void visit_if_stmt(IfStmt *if_stmt)
    {
        // throw BirdException("Implement If Statement");
    }

    void visit_call(Call *call)
    {
        // throw BirdException("Implement visit_call");
    }

    void visit_return_stmt(ReturnStmt *return_stmt)
    {
        // throw BirdException("Implement Return Statement");
    }

    void visit_break_stmt(BreakStmt *break_stmt)
    {
        // throw BirdException("Implement Break Statement");
    }

    void visit_continue_stmt(ContinueStmt *continue_stmt)
    {
        // throw BirdException("Implement Continue Statement");
    }
};

// #include <memory>
// #include <vector>
// #include <map>
// #include <string>
// #include "../ast_node/stmt/stmt.h"
// #include "../ast_node/expr/expr.h"

// #include "../ast_node/expr/binary.h"
// #include "../ast_node/expr/unary.h"
// #include "../ast_node/expr/primary.h"
// #include "../ast_node/expr/ternary.h"
// #include "../ast_node/expr/call.h"

// #include "../ast_node/stmt/decl_stmt.h"
// #include "../ast_node/expr/assign_expr.h"
// #include "../ast_node/stmt/expr_stmt.h"
// #include "../ast_node/stmt/print_stmt.h"
// #include "../ast_node/stmt/while_stmt.h"
// #include "../ast_node/stmt/for_stmt.h"
// #include "../ast_node/stmt/return_stmt.h"
// #include "../ast_node/stmt/break_stmt.h"
// #include "../ast_node/stmt/continue_stmt.h"
// #include "../ast_node/stmt/if_stmt.h"
// #include "../ast_node/stmt/block.h"

// #include "../exceptions/bird_exception.h"
// #include "../exceptions/return_exception.h"
// #include "../exceptions/break_exception.h"
// #include "../exceptions/continue_exception.h"
// #include "../sym_table.h"

// #include "llvm/ADT/APFloat.h"
// #include "llvm/ADT/STLExtras.h"
// #include "llvm/IR/BasicBlock.h"
// #include "llvm/IR/Constants.h"
// #include "llvm/IR/DerivedTypes.h"
// #include "llvm/IR/Function.h"
// #include "llvm/IR/IRBuilder.h"
// #include "llvm/IR/Instructions.h"
// #include "llvm/IR/LLVMContext.h"
// #include "llvm/IR/LegacyPassManager.h"
// #include "llvm/IR/Module.h"
// #include "llvm/IR/Type.h"
// #include "llvm/IR/Verifier.h"
// #include "llvm/MC/TargetRegistry.h"
// #include "llvm/Support/FileSystem.h"
// #include "llvm/Support/TargetSelect.h"
// #include "llvm/Support/raw_ostream.h"
// #include "llvm/Target/TargetMachine.h"
// #include "llvm/Target/TargetOptions.h"
// #include "llvm/TargetParser/Host.h"

// /*
//  * Visitor that generates the LLVM IR representation of the code
//  */
// class CodeGen : public Visitor
// {
//     std::stack<llvm::Value *> stack;
//     std::shared_ptr<SymbolTable<llvm::AllocaInst *>> environment;
//     std::map<std::string, llvm::FunctionCallee> std_lib;

//     llvm::LLVMContext context;
//     llvm::IRBuilder<> builder;
//     std::map<std::string, llvm::Value *> format_strings;

//     llvm::BasicBlock *current_condition_block;
//     llvm::BasicBlock *current_done_block;

// public:
//     std::unique_ptr<llvm::Module> module;

//     ~CodeGen()
//     {
//         while (!this->stack.empty())
//         {
//             auto val = this->stack.top();
//             this->stack.pop();
//             free(val);
//         }
//     }

//     CodeGen() : builder(llvm::IRBuilder<>(this->context))
//     {
//         this->environment = std::make_shared<SymbolTable<llvm::AllocaInst *>>(SymbolTable<llvm::AllocaInst *>());
//         this->module = std::make_unique<llvm::Module>("test_module", this->context);
//     }

//     void init_std_lib()
//     {
//         // Declare the printf function (external function)
//         // How can we do this better?
//         llvm::FunctionType *printfType = llvm::FunctionType::get(this->builder.getInt32Ty(), this->builder.getPtrTy(), true);
//         llvm::FunctionCallee printfFunc = this->module->getOrInsertFunction("printf", printfType);
//         this->std_lib["print"] = printfFunc;
//         // TODO: when to free printf?
//     }

//     llvm::BasicBlock *create_entry_point()
//     {
//         // main function
//         llvm::FunctionType *funcType = llvm::FunctionType::get(this->builder.getVoidTy(), false);
//         llvm::Function *mainFunction = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, "main", this->module.get());
//         llvm::BasicBlock *entry = llvm::BasicBlock::Create(this->context, "entry", mainFunction);

//         // TODO: when to free entry block?
//         return entry;
//     }

//     void generate(std::vector<std::unique_ptr<Stmt>> *stmts)
//     {
//         this->init_std_lib();
//         auto entry = this->create_entry_point();

//         this->builder.SetInsertPoint(entry);

//         this->format_strings["int"] = this->builder.CreateGlobalString("%d");
//         this->format_strings["float"] = this->builder.CreateGlobalString("%f");
//         this->format_strings["str"] = this->builder.CreateGlobalString("%s");
//         this->format_strings["newline"] = this->builder.CreateGlobalString("\n");

//         for (auto &stmt : *stmts)
//         {
//             if (auto func_stmt = dynamic_cast<Func *>(stmt.get()))
//             {
//                 func_stmt->accept(this);
//             }

//             if (auto decl_stmt = dynamic_cast<DeclStmt *>(stmt.get()))
//             {
//                 decl_stmt->accept(this);
//             }

//             if (auto print_stmt = dynamic_cast<PrintStmt *>(stmt.get()))
//             {
//                 print_stmt->accept(this);
//             }

//             if (auto if_stmt = dynamic_cast<IfStmt *>(stmt.get()))
//             {
//                 if_stmt->accept(this);
//             }

//             if (auto block = dynamic_cast<Block *>(stmt.get()))
//             {
//                 block->accept(this);
//             }

//             if (auto expr_stmt = dynamic_cast<ExprStmt *>(stmt.get()))
//             {
//                 expr_stmt->accept(this);
//             }

//             if (auto ternary_stmt = dynamic_cast<Ternary *>(stmt.get()))
//             {
//                 ternary_stmt->accept(this);
//             }

//             if (auto while_stmt = dynamic_cast<WhileStmt *>(stmt.get()))
//             {
//                 while_stmt->accept(this);
//             }

//             if (auto for_stmt = dynamic_cast<ForStmt *>(stmt.get()))
//             {
//                 for_stmt->accept(this);
//             }

//             if (auto return_stmt = dynamic_cast<ReturnStmt *>(stmt.get()))
//             {
//                 return_stmt->accept(this);
//             }

//             if (auto break_stmt = dynamic_cast<BreakStmt *>(stmt.get()))
//             {
//                 break_stmt->accept(this);
//             }

//             if (auto continue_stmt = dynamic_cast<ContinueStmt *>(stmt.get()))
//             {
//                 continue_stmt->accept(this);
//             }
//         }

//         this->builder.CreateRetVoid();

//         this->module->print(llvm::outs(), nullptr);

//         while (!this->stack.empty())
//         {
//             this->stack.pop();
//         }

//         std::error_code ll_EC;
//         llvm::raw_fd_ostream ll_dest("output.ll", ll_EC);

//         if (ll_EC)
//         {
//             llvm::errs() << "Could not open file: " << ll_EC.message();
//             exit(1);
//         }

//         this->module->print(ll_dest, nullptr);
//         ll_dest.close();

//         auto TargetTriple = llvm::Triple("wasm32-unknown-unknown");

//         llvm::InitializeAllTargetInfos();
//         llvm::InitializeAllTargets();
//         llvm::InitializeAllTargetMCs();
//         llvm::InitializeAllAsmParsers();
//         llvm::InitializeAllAsmPrinters();

//         std::string Error;
//         auto Target = llvm::TargetRegistry::lookupTarget(TargetTriple.getTriple(), Error);

//         if (!Target)
//         {
//             llvm::errs() << Error;
//             return;
//         }

//         auto CPU = "generic";
//         auto Features = "";

//         llvm::TargetOptions opt;
//         auto TargetMachine = Target->createTargetMachine(TargetTriple.getTriple(), CPU, Features, opt, llvm::Reloc::PIC_);

//         this->module->setDataLayout(TargetMachine->createDataLayout());
//         this->module->setTargetTriple(TargetTriple.getTriple());

//         auto Filename = "output.o";
//         std::error_code EC;
//         llvm::raw_fd_ostream dest(Filename, EC, llvm::sys::fs::OF_None);

//         if (EC)
//         {
//             llvm::errs() << "Could not open file: " << EC.message();
//             return;
//         }

//         llvm::legacy::PassManager pass;
//         auto FileType = llvm::CodeGenFileType::ObjectFile;

//         if (TargetMachine->addPassesToEmitFile(pass, dest, nullptr, FileType))
//         {
//             llvm::errs() << "TargetMachine can't emit a file of this type";
//             return;
//         }

//         pass.run(*this->module);
//         dest.flush();
//     }

//     llvm::Type *from_bird_type(Token token)
//     {
//         if (token.lexeme == "bool")
//             return llvm::Type::getInt1Ty(context);
//         else if (token.lexeme == "int")
//             return llvm::Type::getInt32Ty(context);
//         else if (token.lexeme == "float")
//             return llvm::Type::getDoubleTy(context);
//         else if (token.lexeme == "void")
//             return llvm::Type::getVoidTy(context);
//         else if (token.lexeme == "str")
//             return llvm::Type::getInt8Ty(this->context)->getPointerTo();
//         else
//             throw BirdException("invalid type");
//     }

//     void visit_block(Block *block)
//     {
//         auto new_environment = std::make_shared<SymbolTable<llvm::AllocaInst *>>(SymbolTable<llvm::AllocaInst *>());
//         new_environment->set_enclosing(this->environment);
//         this->environment = new_environment;

//         for (auto &stmt : block->stmts)
//         {
//             stmt->accept(this);
//         }

//         this->environment = this->environment->get_enclosing();
//     }

//     void visit_decl_stmt(DeclStmt *decl_stmt)
//     {
//         decl_stmt->value->accept(this);

//         llvm::Value *initializer_value = this->stack.top();
//         this->stack.pop();

//         llvm::Function *function = this->builder.GetInsertBlock()->getParent();

//         llvm::AllocaInst *alloca = this->builder.CreateAlloca(initializer_value->getType(), nullptr, decl_stmt->identifier.lexeme.c_str());
//         this->builder.CreateStore(initializer_value, alloca);

//         this->environment->insert(decl_stmt->identifier.lexeme, alloca);
//     }

//     void visit_assign_expr(AssignExpr *assign_expr)
//     {
//         auto lhs = this->environment->get(assign_expr->identifier.lexeme);
//         auto lhs_val = this->builder.CreateLoad(lhs->getAllocatedType(), lhs, "loadtmp");

//         assign_expr->value->accept(this);
//         auto rhs_val = this->stack.top();
//         this->stack.pop();

//         bool float_flag = (lhs_val->getType()->isDoubleTy() || rhs_val->getType()->isDoubleTy());

//         if (float_flag && lhs_val->getType()->isIntegerTy())
//         {
//             rhs_val = this->builder.CreateFPToSI(rhs_val, lhs_val->getType(), "floattoint");
//             float_flag = false;
//         }
//         else if (float_flag && lhs_val->getType()->isDoubleTy())
//         {
//             rhs_val = this->builder.CreateSIToFP(rhs_val, lhs_val->getType(), "inttofloat");
//         }

//         llvm::Value *result = nullptr;
//         switch (assign_expr->assign_operator.token_type)
//         {
//         case Token::Type::EQUAL:
//         {
//             result = rhs_val;
//             break;
//         }
//         case Token::Type::PLUS_EQUAL:
//         {
//             result = (float_flag)
//                          ? this->builder.CreateFAdd(lhs_val, rhs_val, "faddtmp")
//                          : this->builder.CreateAdd(lhs_val, rhs_val, "addtmp");
//             break;
//         }
//         case Token::Type::MINUS_EQUAL:
//         {
//             result = (float_flag)
//                          ? this->builder.CreateFSub(lhs_val, rhs_val, "fsubtmp")
//                          : this->builder.CreateSub(lhs_val, rhs_val, "subtmp");
//             break;
//         }
//         case Token::Type::STAR_EQUAL:
//         {
//             result = (float_flag)
//                          ? this->builder.CreateFMul(lhs_val, rhs_val, "fmultmp")
//                          : this->builder.CreateMul(lhs_val, rhs_val, "multmp");
//             break;
//         }
//         case Token::Type::SLASH_EQUAL:
//         {
//             result = (float_flag)
//                          ? this->builder.CreateFDiv(lhs_val, rhs_val, "fdivtmp")
//                          : this->builder.CreateSDiv(lhs_val, rhs_val, "divtmp");
//             break;
//         }
//         case Token::Type::PERCENT_EQUAL:
//         {
//             result = (float_flag)
//                          ? throw BirdException("Modular operation requires integer values")
//                          : this->builder.CreateSRem(lhs_val, rhs_val, "modtmp");
//             break;
//         }
//         default:
//             throw BirdException("Unidentified assignment operator " + assign_expr->assign_operator.lexeme);
//             break;
//         }

//         this->builder.CreateStore(result, lhs);
//     }

//     void visit_print_stmt(PrintStmt *print_stmt)
//     {
//         for (auto &arg : print_stmt->args)
//         {
//             arg->accept(this);
//         }

//         // TODO: figure out a better way to do this
//         auto printfFunc = this->std_lib["print"];

//         std::vector<llvm::Value *> results;
//         for (int i = 0; i < print_stmt->args.size(); i++)
//         {
//             results.push_back(this->stack.top());
//             this->stack.pop();
//         }

//         for (int i = 0; i < results.size(); i++)
//         {
//             auto result = results[results.size() - 1 - i];
//             llvm::Value *formatStr;
//             if (result->getType()->isIntegerTy())
//             {
//                 formatStr = this->format_strings["int"];
//             }
//             else if (result->getType()->isDoubleTy())
//             {
//                 formatStr = this->format_strings["float"];
//                 result = builder.CreateFPExt(result, llvm::Type::getDoubleTy(context));
//             }
//             else if (result->getType()->isPointerTy())
//             {
//                 formatStr = this->format_strings["str"];
//             }
//             this->builder.CreateCall(printfFunc, {formatStr, result});
//         }

//         this->builder.CreateCall(printfFunc, {this->format_strings["newline"]});
//     }

//     void visit_expr_stmt(ExprStmt *expr_stmt)
//     {
//         expr_stmt->expr->accept(this);
//     }

//     void visit_while_stmt(WhileStmt *while_stmt)
//     {
//         auto parent_fn = this->builder.GetInsertBlock()->getParent();

//         auto condition_block = llvm::BasicBlock::Create(this->context, "while_condition", parent_fn);
//         auto stmt_block = llvm::BasicBlock::Create(this->context, "while_stmt", parent_fn);
//         auto done_block = llvm::BasicBlock::Create(this->context, "while_done", parent_fn);

//         auto old_condition_block = this->current_condition_block;
//         auto old_done_block = this->current_done_block;

//         this->current_condition_block = condition_block;
//         this->current_done_block = done_block;

//         this->builder.CreateBr(condition_block);
//         this->builder.SetInsertPoint(condition_block);

//         while_stmt->condition->accept(this);
//         auto condition = this->stack.top();
//         this->stack.pop();

//         this->builder.CreateCondBr(condition, stmt_block, done_block);

//         this->builder.SetInsertPoint(stmt_block);

//         // try
//         // {
//         while_stmt->stmt->accept(this);
//         this->builder.CreateBr(condition_block);
//         // }
//         // catch (BreakException)
//         // {
//         //     this->builder.CreateBr(done_block);
//         // }
//         // catch (ContinueException)
//         // {
//         //     this->builder.CreateBr(condition_block);
//         // }

//         this->builder.SetInsertPoint(done_block);

//         this->current_condition_block = old_condition_block;
//         this->current_done_block = old_done_block;
//     }

//     void visit_for_stmt(ForStmt *for_stmt)
//     {
//         std::shared_ptr<SymbolTable<llvm::AllocaInst *>> new_environment =
//             std::make_shared<SymbolTable<llvm::AllocaInst *>>();

//         new_environment->set_enclosing(this->environment);
//         this->environment = new_environment;

//         auto parent_fn = this->builder.GetInsertBlock()->getParent();

//         auto init_block = llvm::BasicBlock::Create(this->context, "for_initializer", parent_fn);
//         auto condition_block = llvm::BasicBlock::Create(this->context, "for_condition", parent_fn);
//         auto increment_block = llvm::BasicBlock::Create(this->context, "for_increment", parent_fn);
//         auto body_block = llvm::BasicBlock::Create(this->context, "for_body", parent_fn);
//         auto done_block = llvm::BasicBlock::Create(this->context, "for_merge", parent_fn);

//         auto old_condition_block = this->current_condition_block;
//         auto old_done_block = this->current_done_block;

//         this->current_condition_block = condition_block;
//         this->current_done_block = done_block;

//         this->builder.CreateBr(init_block);
//         this->builder.SetInsertPoint(init_block);

//         if (for_stmt->initializer.has_value())
//         {
//             for_stmt->initializer.value()->accept(this);
//         }

//         this->builder.CreateBr(condition_block);
//         this->builder.SetInsertPoint(condition_block);

//         if (for_stmt->condition.has_value())
//         {
//             for_stmt->condition.value()->accept(this);
//             auto result = this->stack.top();

//             this->builder.CreateCondBr(result, body_block, done_block);
//         }
//         else
//         {
//             this->builder.CreateBr(body_block);
//         }

//         this->builder.SetInsertPoint(body_block);
//         for_stmt->body->accept(this);

//         this->builder.CreateBr(increment_block);
//         this->builder.SetInsertPoint(increment_block);

//         if (for_stmt->increment.has_value())
//         {
//             for_stmt->increment.value()->accept(this);
//         }

//         this->builder.CreateBr(condition_block);
//         this->builder.SetInsertPoint(done_block);

//         this->environment = this->environment->get_enclosing();

//         this->current_condition_block = old_condition_block;
//         this->current_done_block = old_done_block;
//     }

//     void visit_binary(Binary *binary)
//     {
//         binary->left->accept(this);
//         binary->right->accept(this);

//         auto right = this->stack.top();
//         this->stack.pop();

//         auto left = this->stack.top();
//         this->stack.pop();

//         bool float_flag = left->getType()->isDoubleTy();

//         if (left->getType()->isDoubleTy())
//         {
//             if (right->getType()->isIntegerTy())
//                 right = this->builder.CreateSIToFP(right, llvm::Type::getDoubleTy(context));
//         }

//         if (left->getType()->isIntegerTy())
//         {
//             if (right->getType()->isDoubleTy())
//                 right = this->builder.CreateFPToSI(right, llvm::Type::getInt32Ty(context));
//         }

//         switch (binary->op.token_type)
//         {
//         case Token::Type::PLUS:
//         {
//             (float_flag)
//                 ? this->stack.push(this->builder.CreateFAdd(left, right, "faddftmp"))
//                 : this->stack.push(this->builder.CreateAdd(left, right, "addtmp"));

//             break;
//         }
//         case Token::Type::MINUS:
//         {
//             (float_flag)
//                 ? this->stack.push(this->builder.CreateFSub(left, right, "fsubtmp"))
//                 : this->stack.push(this->builder.CreateSub(left, right, "subtmp"));

//             break;
//         }
//         case Token::Type::SLASH:
//         {
//             (float_flag)
//                 ? this->stack.push(this->builder.CreateFDiv(left, right, "fdivtmp"))
//                 : this->stack.push(this->builder.CreateSDiv(left, right, "sdivtmp"));

//             break;
//         }
//         case Token::Type::STAR:
//         {
//             (float_flag)
//                 ? this->stack.push(this->builder.CreateFMul(left, right, "fmultmp"))
//                 : this->stack.push(this->builder.CreateMul(left, right, "smultmp"));

//             break;
//         }
//         case Token::Type::GREATER:
//         {
//             (float_flag)
//                 ? this->stack.push(this->builder.CreateFCmpOGT(left, right, "fogttmp"))
//                 : this->stack.push(this->builder.CreateICmpSGT(left, right, "sgttmp"));

//             break;
//         }
//         case Token::Type::GREATER_EQUAL:
//         {
//             (float_flag)
//                 ? this->stack.push(this->builder.CreateFCmpOGE(left, right, "fogetmp"))
//                 : this->stack.push(this->builder.CreateICmpSGE(left, right, "sgetmp"));

//             break;
//         }
//         case Token::Type::LESS:
//         {
//             (float_flag)
//                 ? this->stack.push(this->builder.CreateFCmpOLT(left, right, "folttmp"))
//                 : this->stack.push(this->builder.CreateICmpSLT(left, right, "slttmp"));

//             break;
//         }
//         case Token::Type::LESS_EQUAL:
//         {
//             (float_flag)
//                 ? this->stack.push(this->builder.CreateFCmpOLE(left, right, "foletmp"))
//                 : this->stack.push(this->builder.CreateICmpSLE(left, right, "sletmp"));

//             break;
//         }
//         case Token::Type::EQUAL_EQUAL:
//         {
//             (float_flag)
//                 ? this->stack.push(this->builder.CreateFCmpOEQ(left, right, "foeqtmp"))
//                 : this->stack.push(this->builder.CreateICmpEQ(left, right, "eqtmp"));

//             break;
//         }
//         case Token::Type::BANG_EQUAL:
//         {
//             (float_flag)
//                 ? this->stack.push(this->builder.CreateFCmpONE(left, right, "fonetmp"))
//                 : this->stack.push(this->builder.CreateICmpNE(left, right, "netmp"));

//             break;
//         }
//         default:
//         {
//             throw BirdException("undefined binary operator for code gen");
//         }
//         }
//     }

//     void visit_unary(Unary *unary)
//     {
//         unary->expr->accept(this);
//         auto expr = this->stack.top();
//         this->stack.pop();

//         auto llvm_value = this->builder.CreateNeg(expr);
//         this->stack.push(llvm_value);
//     }

//     void visit_primary(Primary *primary)
//     {
//         switch (primary->value.token_type)
//         {
//         case Token::Type::INT_LITERAL:
//         {
//             int value = std::stoi(primary->value.lexeme);

//             auto llvm_value = llvm::ConstantInt::get(this->context, llvm::APInt(32, value));
//             this->stack.push(llvm_value);
//             break;
//         }
//         case Token::Type::FLOAT_LITERAL:
//         {
//             double value = std::stod(primary->value.lexeme);

//             auto llvm_value = llvm::ConstantFP::get(this->context, llvm::APFloat(value));
//             this->stack.push(llvm_value);
//             break;
//         }
//         case Token::Type::BOOL_LITERAL:
//             this->stack.push(
//                 primary->value.lexeme == "true" ? this->builder.getTrue() : this->builder.getFalse());
//             break;
//         case Token::Type::STR_LITERAL:
//         {
//             auto value = primary->value.lexeme;

//             auto alloca = this->builder.CreateAlloca(
//                 llvm::ArrayType::get(llvm::Type::getInt8Ty(this->context), value.size() + 1),
//                 nullptr, "strtmp");

//             this->builder.CreateStore(llvm::ConstantDataArray::getString(this->context, value, true), alloca);

//             auto pointer = this->builder.CreateBitCast(alloca,
//                                                        llvm::Type::getInt8Ty(this->context)->getPointerTo());

//             this->stack.push(pointer);
//             break;
//         }
//         case Token::Type::IDENTIFIER:
//         {
//             auto allocation = this->environment->get(primary->value.lexeme);
//             if (allocation == nullptr)
//             {
//                 throw BirdException("undefined type identifier");
//             }

//             auto value =
//                 this->builder.CreateLoad(allocation->getAllocatedType(), allocation, primary->value.lexeme.c_str());

//             this->stack.push(value);
//             break;
//         }
//         default:
//         {
//             throw BirdException("undefined primary value");
//         }
//         }
//     }

//     void visit_ternary(Ternary *ternary)
//     {
//         ternary->condition->accept(this);
//         // condition contains LLVM IR i1 bool
//         auto condition = this->stack.top();
//         this->stack.pop();

//         // control flow requires parent
//         auto parent_fn = this->builder.GetInsertBlock()->getParent();

//         // create blocks for true and false expressions in the ternary operation, and a done block to merge control flow
//         auto true_block = llvm::BasicBlock::Create(this->context, "true", parent_fn);
//         auto false_block = llvm::BasicBlock::Create(this->context, "false", parent_fn);
//         auto done_block = llvm::BasicBlock::Create(this->context, "done", parent_fn);

//         // create conditional branch for condition, if true -> true block, if false -> false block
//         this->builder.CreateCondBr(condition, true_block, false_block);

//         this->builder.SetInsertPoint(true_block); // set insert point to true block for true expression instuctions
//         ternary->true_expr->accept(this);
//         auto true_result = this->stack.top();
//         this->stack.pop();

//         this->builder.CreateBr(done_block); // create branch to merge control flow

//         // update true block for phi node reference to predecessor
//         true_block = this->builder.GetInsertBlock();

//         this->builder.SetInsertPoint(false_block); // set insert point for false block for false expression instructions
//         ternary->false_expr->accept(this);
//         auto false_result = this->stack.top();
//         this->stack.pop();

//         this->builder.CreateBr(done_block);

//         // update false block for phi node reference to predecessor
//         false_block = this->builder.GetInsertBlock();

//         // set insert point for done (merge) block
//         this->builder.SetInsertPoint(done_block);

//         /*
//          * phi node which has type `result`, 2 incoming values, and is named "ternary result"
//          * this is how we select the correct result based on which branch is executed
//          */
//         auto phi_node = this->builder.CreatePHI(true_result->getType(), 2, "ternary result");
//         phi_node->addIncoming(true_result, true_block);   // associates true result to true block
//         phi_node->addIncoming(false_result, false_block); // associates false result to false block

//         this->stack.push(phi_node); // push the phi node w branch result on to the stack
//     }

//     void visit_const_stmt(ConstStmt *const_stmt)
//     {
//         const_stmt->value->accept(this);

//         llvm::Value *initializer_value = this->stack.top();
//         this->stack.pop();

//         llvm::Function *function = this->builder.GetInsertBlock()->getParent();

//         llvm::AllocaInst *alloca = this->builder.CreateAlloca(initializer_value->getType(), nullptr, const_stmt->identifier.lexeme.c_str());

//         this->builder.CreateStore(initializer_value, alloca);

//         this->environment->insert(const_stmt->identifier.lexeme, alloca);
//     }

//     void visit_func(Func *func)
//     {
//         llvm::Function *function = nullptr;

//         std::vector<llvm::Type *> param_types;
//         for (const auto &param : func->param_list)
//         {
//             auto param_type = from_bird_type(param.second);
//             param_types.push_back(param_type);
//         }

//         auto return_type = func->return_type.has_value() ? from_bird_type(func->return_type.value()) : llvm::Type::getVoidTy(context);

//         auto function_type = llvm::FunctionType::get(return_type, param_types, false);

//         function = llvm::Function::Create(function_type, llvm::Function::ExternalLinkage, func->identifier.lexeme, module.get());

//         auto block = llvm::BasicBlock::Create(context, "entry", function);
//         auto old_insert_point = this->builder.GetInsertBlock();
//         this->builder.SetInsertPoint(block);

//         auto new_environment = std::make_shared<SymbolTable<llvm::AllocaInst *>>(SymbolTable<llvm::AllocaInst *>());
//         new_environment->set_enclosing(this->environment);
//         this->environment = new_environment;

//         unsigned i = 0;
//         for (auto &param : function->args())
//         {
//             std::string &param_name = func->param_list[i++].first.lexeme;
//             param.setName(param_name);
//             llvm::AllocaInst *alloca = this->builder.CreateAlloca(param.getType(), nullptr, param_name.c_str());
//             this->builder.CreateStore(&param, alloca);
//             environment->insert(param_name, alloca);
//         }

//         func->block->accept(this);

//         this->environment = this->environment->get_enclosing();

//         // other return types are handled in visit_return_stmt
//         if (!func->return_type.has_value() || func->return_type.value().lexeme == "void")
//         {
//             this->builder.CreateRetVoid();
//         }

//         this->builder.SetInsertPoint(old_insert_point);
//     }

//     void visit_if_stmt(IfStmt *if_stmt)
//     {
//         if_stmt->condition->accept(this);

//         auto condition = this->stack.top();
//         this->stack.pop();

//         auto parent_fn = this->builder.GetInsertBlock()->getParent();
//         if (if_stmt->else_branch.has_value())
//         {
//             auto then_block = llvm::BasicBlock::Create(this->context, "then", parent_fn);
//             auto else_block = llvm::BasicBlock::Create(this->context, "else", parent_fn);
//             auto done_block = llvm::BasicBlock::Create(this->context, "if_cont", parent_fn);

//             this->builder.CreateCondBr(condition, then_block, else_block);

//             this->builder.SetInsertPoint(then_block);
//             if_stmt->then_branch->accept(this);
//             this->builder.CreateBr(done_block);

//             this->builder.SetInsertPoint(else_block);
//             if_stmt->else_branch.value().get()->accept(this);

//             this->builder.CreateBr(done_block);
//             this->builder.SetInsertPoint(done_block);
//         }
//         else
//         {
//             auto then_block = llvm::BasicBlock::Create(this->context, "then", parent_fn);
//             auto done_block = llvm::BasicBlock::Create(this->context, "if_cont", parent_fn);

//             this->builder.CreateCondBr(condition, then_block, done_block);

//             this->builder.SetInsertPoint(then_block);
//             if_stmt->then_branch->accept(this);

//             this->builder.CreateBr(done_block);
//             this->builder.SetInsertPoint(done_block);
//         }
//     }

//     void visit_call(Call *call)
//     {
//         auto *function = this->module->getFunction(call->identifier.lexeme);

//         if (!function)
//             throw BirdException("Function '" + call->identifier.lexeme + "' could not be found from callsite");

//         std::vector<llvm::Value *> arguments;

//         for (auto &arg : call->args)
//         {
//             arg->accept(this);
//             arguments.push_back(stack.top());
//             stack.pop();
//         }

//         if (function->getReturnType()->isVoidTy())
//             this->builder.CreateCall(function, arguments);
//         else
//             this->stack.push(this->builder.CreateCall(function, arguments, "calltmp"));
//     }

//     void visit_return_stmt(ReturnStmt *return_stmt)
//     {
//         if (return_stmt->expr.has_value())
//         {
//             return_stmt->expr->get()->accept(this);
//             this->builder.CreateRet(this->stack.top());
//         }
//         else
//         {
//             this->builder.CreateRetVoid();
//         }
//     }

//     void visit_break_stmt(BreakStmt *break_stmt)
//     {
//         this->builder.CreateBr(this->current_done_block);

//         // this is to stop the compiler from complaining about unreachable code
//         llvm::BasicBlock *break_block = llvm::BasicBlock::Create(this->context, "unreachable", this->builder.GetInsertBlock()->getParent());
//         this->builder.SetInsertPoint(break_block);
//     }

//     void visit_continue_stmt(ContinueStmt *continue_stmt)
//     {
//         this->builder.CreateBr(this->current_condition_block);

//         // this is to stop the compiler from complaining about unreachable code
//         llvm::BasicBlock *break_block = llvm::BasicBlock::Create(this->context, "unreachable", this->builder.GetInsertBlock()->getParent());
//         this->builder.SetInsertPoint(break_block);
//     }
// };
