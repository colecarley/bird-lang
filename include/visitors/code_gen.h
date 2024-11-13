#pragma once
#include "binaryen-c.h"

enum CodeGenType {
    CodeGenInt,
    CodeGenFloat,
    CodeGenBool,
    CodeGenVoid,
    CodeGenPtr
};

struct CodeGenIndex {
    BinaryenIndex index;
    std::optional<CodeGenType> type;

    CodeGenIndex() : index(0), type(std::nullopt) {}
    CodeGenIndex(BinaryenIndex indexP, std::optional<CodeGenType> typeP = std::nullopt)
        : index(indexP), type(typeP) {}
};

struct CodeGenExpression {
    BinaryenExpressionRef expression;
    std::optional<CodeGenType> type;

    CodeGenExpression(BinaryenExpressionRef expressionP, std::optional<CodeGenType> typeP = std::nullopt)
        : expression(expressionP), type(typeP) {}
};

struct CodeGenBinaryenType {
    BinaryenType binaryen_type;
    std::optional<CodeGenType> type;

    CodeGenBinaryenType() : binaryen_type(BinaryenTypeInt32()), type(std::nullopt) {}
    CodeGenBinaryenType(BinaryenType binaryen_typeP, std::optional<CodeGenType> typeP = std::nullopt)
        : binaryen_type(binaryen_typeP), type(typeP) {}
};

class CodeGen : public Visitor
{
    Environment<CodeGenIndex> environment; // tracks the index of local variables
    Stack<CodeGenExpression> stack;     // for returning values

    std::map<std::string, std::string> std_lib;

    // we need the function return types when calling functions
    std::unordered_map<std::string, CodeGenBinaryenType> function_return_types;
    // allows us to track the local variables of a function
    std::unordered_map<std::string, std::vector<CodeGenBinaryenType>> function_locals;
    std::string current_function_name; // for indexing into maps

    BinaryenModuleRef mod;
    int heap_offset;

public:
    ~CodeGen()
    {
        BinaryenModuleDispose(this->mod);
    }

    CodeGen() : mod(BinaryenModuleCreate())
    {
        this->environment = Environment<CodeGenIndex>();
        BinaryenSetMemory(this->mod, 1, 1, "memory", nullptr, nullptr, nullptr, nullptr, 0, 0);
        this->heap_offset = 0;
    }

    void init_std_lib()
    {
        BinaryenAddFunctionImport(
            this->mod,
            "print",
            "env",
            "print",
            BinaryenTypeCreate((BinaryenType[]){BinaryenTypeInt32()}, 1),
            BinaryenTypeNone());
    }

    void generate(std::vector<std::unique_ptr<Stmt>> *stmts)
    {
        this->init_std_lib();
        this->environment.push_env();

        this->current_function_name = "main";
        auto main_function_body = std::vector<CodeGenExpression>();
        this->function_locals[this->current_function_name] = std::vector<CodeGenBinaryenType>();

        for (auto &stmt : *stmts)
        {
            if (auto func_stmt = dynamic_cast<Func *>(stmt.get()))
            {
                func_stmt->accept(this);
                // no stack push here, automatically added
            }

            if (auto decl_stmt = dynamic_cast<DeclStmt *>(stmt.get()))
            {
                decl_stmt->accept(this);
                auto result = this->stack.pop();
                main_function_body.push_back(result);
            }

            if (auto print_stmt = dynamic_cast<PrintStmt *>(stmt.get()))
            {
                print_stmt->accept(this);
                auto result = this->stack.pop();
                main_function_body.push_back(result);
            }

            if (auto if_stmt = dynamic_cast<IfStmt *>(stmt.get()))
            {
                if_stmt->accept(this);
                auto result = this->stack.pop();
                main_function_body.push_back(result);
            }

            if (auto block = dynamic_cast<Block *>(stmt.get()))
            {
                block->accept(this);
                auto result = this->stack.pop();
                main_function_body.push_back(result);
            }

            if (auto expr_stmt = dynamic_cast<ExprStmt *>(stmt.get()))
            {
                expr_stmt->accept(this);
                auto result = this->stack.pop();
                main_function_body.push_back(result);
            }

            if (auto ternary_stmt = dynamic_cast<Ternary *>(stmt.get()))
            {
                ternary_stmt->accept(this);
                auto result = this->stack.pop();
                main_function_body.push_back(result);
            }

            if (auto while_stmt = dynamic_cast<WhileStmt *>(stmt.get()))
            {
                while_stmt->accept(this);
                auto result = this->stack.pop();
                main_function_body.push_back(result);
            }

            if (auto for_stmt = dynamic_cast<ForStmt *>(stmt.get()))
            {
                for_stmt->accept(this);
                auto result = this->stack.pop();
                main_function_body.push_back(result);
            }

            if (auto return_stmt = dynamic_cast<ReturnStmt *>(stmt.get()))
            {
                return_stmt->accept(this);
                auto result = this->stack.pop();
                main_function_body.push_back(result);
            }

            if (auto break_stmt = dynamic_cast<BreakStmt *>(stmt.get()))
            {
                break_stmt->accept(this);
                auto result = this->stack.pop();
                main_function_body.push_back(result);
            }

            if (auto continue_stmt = dynamic_cast<ContinueStmt *>(stmt.get()))
            {
                continue_stmt->accept(this);
                auto result = this->stack.pop();
                main_function_body.push_back(result);
            }
        }

        BinaryenType params = BinaryenTypeNone();
        BinaryenType results = BinaryenTypeNone();

        std::vector<BinaryenExpressionRef> binaryen_expressions;
        for (CodeGenExpression code_gen_expression : main_function_body) {
            binaryen_expressions.push_back(code_gen_expression.expression);
        }

        std::vector<BinaryenType> binaryen_function_locals;
        for (CodeGenBinaryenType code_gen_binaryen_type : this->function_locals["main"]) {
            binaryen_function_locals.push_back(code_gen_binaryen_type.binaryen_type);
        }

        BinaryenExpressionRef body =
            BinaryenBlock(
                this->mod,
                nullptr,
                binaryen_expressions.data(),
                binaryen_expressions.size(),
                BinaryenTypeNone());

        BinaryenFunctionRef mainFunction =
            BinaryenAddFunction(
                this->mod,
                "main",
                params,
                results,
                binaryen_function_locals.data(),
                binaryen_function_locals.size(),
                body);

        BinaryenAddFunctionExport(
            this->mod,
            "main",
            "main");

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

        this->environment.pop_env();
    }

    BinaryenType bird_type_to_binaryen_type(Token token)
    {
        if (token.lexeme == "bool")
            return BinaryenTypeInt32(); // bool is represented as i32
        else if (token.lexeme == "int")
            return BinaryenTypeInt32();
        else if (token.lexeme == "float")
            return BinaryenTypeFloat64();
        else if (token.lexeme == "void")
            return BinaryenTypeNone();
        else if (token.lexeme == "str")
            return BinaryenTypeInt32();
        else
            throw BirdException("invalid type");
    }

    CodeGenType bird_type_to_code_gen_type(Token token)
    {
        if (token.lexeme == "bool")
            return CodeGenBool;
        else if (token.lexeme == "int")
            return CodeGenInt;
        else if (token.lexeme == "float")
            return CodeGenFloat;
        else if (token.lexeme == "void")
            return CodeGenVoid;
        else if (token.lexeme == "str")
            return CodeGenPtr;
        else
            throw BirdException("invalid type");
    }

    void visit_block(Block *block)
    {
        std::vector<BinaryenExpressionRef> children;
        this->environment.push_env();

        for (auto &stmt : block->stmts)
        {
            stmt->accept(this);
            auto result = this->stack.pop();

            if (result.expression)
            {
                children.push_back(result.expression);
            }
        }

        this->environment.pop_env();

        BinaryenExpressionRef block_expr =
            BinaryenBlock(
                this->mod,
                nullptr,
                children.data(),
                children.size(),
                BinaryenTypeNone());

        this->stack.push(CodeGenExpression(block_expr));
    }

    void visit_decl_stmt(DeclStmt *decl_stmt)
    {
        decl_stmt->value->accept(this);
        CodeGenExpression initializer_value = this->stack.pop();

        BinaryenType binaryen_type = decl_stmt->type_identifier.has_value()
                                ? bird_type_to_binaryen_type(decl_stmt->type_identifier.value())
                                : BinaryenExpressionGetType(initializer_value.expression);

        BinaryenIndex index = this->function_locals[this->current_function_name].size();
        this->function_locals[this->current_function_name].push_back(CodeGenBinaryenType(binaryen_type, initializer_value.type));

        BinaryenExpressionRef set_local = BinaryenLocalSet(this->mod, index, initializer_value.expression);
        environment.declare(decl_stmt->identifier.lexeme, CodeGenIndex(index, initializer_value.type));

        this->stack.push(CodeGenExpression(set_local));
    }

    void visit_assign_expr(AssignExpr *assign_expr)
    {
        CodeGenIndex code_gen_index = this->environment.get(assign_expr->identifier.lexeme);
        CodeGenBinaryenType function_local = this->function_locals[this->current_function_name][code_gen_index.index];

        CodeGenExpression lhs_val = CodeGenExpression(
            BinaryenLocalGet(
                this->mod,
                code_gen_index.index,
                function_local.binaryen_type
            ),
            function_local.type
        );

        assign_expr->value->accept(this);
        CodeGenExpression rhs_val = this->stack.pop();

        bool float_flag = (lhs_val.type == CodeGenFloat ||
                           rhs_val.type == CodeGenFloat);

        if (float_flag && lhs_val.type == CodeGenInt)
        {
            rhs_val = CodeGenExpression(
                BinaryenUnary(
                    mod,
                    BinaryenTruncSatSFloat64ToInt32(),
                    rhs_val.expression
                ),
                CodeGenInt
            );
        }
        else if (float_flag && rhs_val.type == CodeGenInt)
        {
            rhs_val = CodeGenExpression(
                BinaryenUnary(
                    mod,
                    BinaryenConvertSInt32ToFloat64(),
                    rhs_val.expression
                ),
                CodeGenInt
            );
        }

        BinaryenExpressionRef result;
        switch (assign_expr->assign_operator.token_type)
        {
        case Token::Type::EQUAL:
        {
            result = rhs_val.expression;
            break;
        }
        case Token::Type::PLUS_EQUAL:
        {
            // TODO: figure out string conatenation
            result = (float_flag)
                         ? BinaryenBinary(this->mod, BinaryenAddFloat64(), lhs_val.expression, rhs_val.expression)
                         : BinaryenBinary(this->mod, BinaryenAddInt32(), lhs_val.expression, rhs_val.expression);

            break;
        }
        case Token::Type::MINUS_EQUAL:
        {
            result = (float_flag)
                         ? BinaryenBinary(this->mod, BinaryenSubFloat64(), lhs_val.expression, rhs_val.expression)
                         : BinaryenBinary(this->mod, BinaryenSubInt32(), lhs_val.expression, rhs_val.expression);

            break;
        }
        case Token::Type::STAR_EQUAL:
        {
            result = (float_flag)
                         ? BinaryenBinary(this->mod, BinaryenMulFloat64(), lhs_val.expression, rhs_val.expression)
                         : BinaryenBinary(this->mod, BinaryenMulInt32(), lhs_val.expression, rhs_val.expression);

            break;
        }
        case Token::Type::SLASH_EQUAL:
        {
            result = (float_flag)
                         ? BinaryenBinary(this->mod, BinaryenDivFloat64(), lhs_val.expression, rhs_val.expression)
                         : BinaryenBinary(this->mod, BinaryenDivSInt32(), lhs_val.expression, rhs_val.expression);

            break;
        }
        case Token::Type::PERCENT_EQUAL:
        {
            result = (float_flag)
                         ? throw BirdException("Modular operation requires integer values")
                         : BinaryenBinary(this->mod, BinaryenRemSInt32(), lhs_val.expression, rhs_val.expression);

            break;
        }
        default:
            throw BirdException("Unidentified assignment operator " + assign_expr->assign_operator.lexeme);
            break;
        }

        BinaryenExpressionRef assign_stmt = BinaryenLocalSet(
            this->mod,
            code_gen_index.index,
            result);

        this->stack.push(CodeGenExpression(assign_stmt));
    }

    void visit_print_stmt(PrintStmt *print_stmt)
    {
        for (auto &arg : print_stmt->args)
        {
            arg->accept(this);
        }

        std::vector<BinaryenExpressionRef> results;
        for (int i = 0; i < print_stmt->args.size(); i++)
        {
            results.push_back(this->stack.pop().expression);
        }

        std::vector<BinaryenExpressionRef> console_log_args;
        for (auto &result : results)
        {
            if (BinaryenExpressionGetType(result) == BinaryenTypeInt32())
            {
                console_log_args.push_back(result);
            }
        }

        BinaryenExpressionRef consoleLogCall =
            BinaryenCall(
                this->mod,
                "print",
                console_log_args.data(),
                console_log_args.size(),
                BinaryenTypeNone());

        this->stack.push(CodeGenExpression(consoleLogCall));
    }

    void visit_expr_stmt(ExprStmt *expr_stmt)
    {
        expr_stmt->expr->accept(this);
        // pop and push it back to the stack
    }

    void visit_while_stmt(WhileStmt *while_stmt)
    {
    }

    void visit_for_stmt(ForStmt *for_stmt)
    {
    }

    void visit_binary(Binary *binary)
    {
        binary->left->accept(this);
        binary->right->accept(this);

        auto right = this->stack.pop();
        auto left = this->stack.pop();

        bool float_flag = (left.type == CodeGenFloat ||
                           right.type == CodeGenFloat);

        if (float_flag && left.type == CodeGenInt)
        {
            left =
                BinaryenUnary(
                    mod,
                    BinaryenConvertSInt32ToFloat64(),
                    left.expression);
        }
        else if (float_flag && right.type == CodeGenInt)
        {
            right =
                BinaryenUnary(
                    mod,
                    BinaryenConvertSInt32ToFloat64(),
                    right.expression);
        }

        switch (binary->op.token_type)
        {
        case Token::Type::PLUS:
        {
            (float_flag)
                ? this->stack.push(CodeGenExpression(BinaryenBinary(mod, BinaryenAddFloat64(), left.expression, right.expression)))
                : this->stack.push(CodeGenExpression(BinaryenBinary(mod, BinaryenAddInt32(), left.expression, right.expression)));

            break;
        }
        case Token::Type::MINUS:
        {
            (float_flag)
                ? this->stack.push(CodeGenExpression(BinaryenBinary(mod, BinaryenSubFloat64(), left.expression, right.expression)))
                : this->stack.push(CodeGenExpression(BinaryenBinary(mod, BinaryenSubInt32(), left.expression, right.expression)));

            break;
        }
        case Token::Type::SLASH:
        {
            (float_flag)
                ? this->stack.push(CodeGenExpression(BinaryenBinary(mod, BinaryenDivFloat64(), left.expression, right.expression)))
                : this->stack.push(CodeGenExpression(BinaryenBinary(mod, BinaryenDivSInt32(), left.expression, right.expression)));

            break;
        }
        case Token::Type::STAR:
        {
            (float_flag)
                ? this->stack.push(CodeGenExpression(BinaryenBinary(mod, BinaryenMulFloat64(), left.expression, right.expression)))
                : this->stack.push(CodeGenExpression(BinaryenBinary(mod, BinaryenMulInt32(), left.expression, right.expression)));

            break;
        }
        case Token::Type::GREATER:
        {
            (float_flag)
                ? this->stack.push(CodeGenExpression(BinaryenBinary(mod, BinaryenGtFloat64(), left.expression, right.expression)))
                : this->stack.push(CodeGenExpression(BinaryenBinary(mod, BinaryenGtSInt32(), left.expression, right.expression)));

            break;
        }
        case Token::Type::GREATER_EQUAL:
        {
            (float_flag)
                ? this->stack.push(CodeGenExpression(BinaryenBinary(mod, BinaryenGeFloat64(), left.expression, right.expression)))
                : this->stack.push(CodeGenExpression(BinaryenBinary(mod, BinaryenGeSInt32(), left.expression, right.expression)));

            break;
        }
        case Token::Type::LESS:
        {
            (float_flag)
                ? this->stack.push(CodeGenExpression(BinaryenBinary(mod, BinaryenLtFloat64(), left.expression, right.expression)))
                : this->stack.push(CodeGenExpression(BinaryenBinary(mod, BinaryenLtSInt32(), left.expression, right.expression)));

            break;
        }
        case Token::Type::LESS_EQUAL:
        {
            (float_flag)
                ? this->stack.push(CodeGenExpression(BinaryenBinary(mod, BinaryenLeFloat64(), left.expression, right.expression)))
                : this->stack.push(CodeGenExpression(BinaryenBinary(mod, BinaryenLeSInt32(), left.expression, right.expression)));

            break;
        }
        case Token::Type::EQUAL_EQUAL:
        {
            (float_flag)
                ? this->stack.push(CodeGenExpression(BinaryenBinary(mod, BinaryenEqFloat64(), left.expression, right.expression)))
                : this->stack.push(CodeGenExpression(BinaryenBinary(mod, BinaryenEqInt32(), left.expression, right.expression)));

            break;
        }
        case Token::Type::BANG_EQUAL:
        {
            (float_flag)
                ? this->stack.push(CodeGenExpression(BinaryenBinary(mod, BinaryenNeFloat64(), left.expression, right.expression)))
                : this->stack.push(CodeGenExpression(BinaryenBinary(mod, BinaryenNeInt32(), left.expression, right.expression)));

            break;
        }
        default:
        {
            throw BirdException("undefined binary operator for code gen");
        }
        }
    }

    /*
     * Binaryen doesnt support integer negation for some reason,
     * only float32 and 64, so i guess the work around would be
     * to subtract it from zero.
     */
    void visit_unary(Unary *unary)
    {
        unary->expr->accept(this);
        auto code_gen_expression = this->stack.pop();

        if (code_gen_expression.type == CodeGenFloat)
        {
            this->stack.push(CodeGenExpression(BinaryenUnary(mod, BinaryenNegFloat64(), code_gen_expression.expression), CodeGenFloat));
        }
        else if (code_gen_expression.type == CodeGenInt)
        {
            BinaryenExpressionRef zero = BinaryenConst(mod, BinaryenLiteralInt32(0));

            this->stack.push(
                CodeGenExpression(BinaryenBinary(
                    mod,
                    BinaryenSubInt32(),
                    zero,
                    code_gen_expression.expression), CodeGenInt));
        }
    }

    void visit_primary(Primary *primary)
    {
        switch (primary->value.token_type)
        {
        case Token::Type::INT_LITERAL:
        {
            int value = std::stoi(primary->value.lexeme);
            BinaryenExpressionRef int_literal = BinaryenConst(this->mod, BinaryenLiteralInt32(value));

            this->stack.push(CodeGenExpression(int_literal, CodeGenInt));
            break;
        }

        case Token::Type::FLOAT_LITERAL:
        {
            double value = std::stod(primary->value.lexeme);
            BinaryenExpressionRef float_literal = BinaryenConst(this->mod, BinaryenLiteralFloat64(value));

            this->stack.push(CodeGenExpression(float_literal, CodeGenFloat));
            break;
        }

        case Token::Type::BOOL_LITERAL:
        {
            BinaryenExpressionRef bool_literal = BinaryenConst(
                this->mod,
                primary->value.lexeme == "true" ? BinaryenLiteralInt32(1) : BinaryenLiteralInt32(0)
            );

            this->stack.push(CodeGenExpression(bool_literal, CodeGenBool));
            break;
        }

        case Token::Type::STR_LITERAL:
        {
            // std::string string_to_store = primary->value.lexeme + "\0";
            // BinaryenExpressionRef string_ptr = BinaryenConst(this->mod, BinaryenLiteralInt32(this->heap_offset));

            // uint32_t bytes = 1;
            // uint32_t align = 1;

            // for (int i = 0; i < string_to_store.size(); i++) {
            //     uint8_t char_to_store = string_to_store[i];
            //     BinaryenExpressionRef ptr = BinaryenConst(this->mod, BinaryenLiteralInt32(this->heap_offset));
            //     BinaryenExpressionRef value = BinaryenConst(this->mod, BinaryenLiteralInt32(char_to_store));

            //     BinaryenExpressionRef store = BinaryenStore(
            //         this->mod,
            //         bytes,
            //         this->heap_offset,
            //         align,
            //         ptr,
            //         value,
            //         BinaryenTypeInt32()
            //     );

            //     this->heap_offset += 1;
            // }
            
            // this->stack.push(CodeGenExpression(string_ptr, CodeGenPtr));
            break;
        }

        case Token::Type::IDENTIFIER:
        {
            CodeGenIndex code_gen_index = this->environment.get(primary->value.lexeme);

            BinaryenExpressionRef identifier = BinaryenLocalGet(
                this->mod,
                code_gen_index.index,
                this->function_locals[this->current_function_name][code_gen_index.index].binaryen_type
            );

            this->stack.push(CodeGenExpression(identifier));
            break;
        }

        default:
            throw BirdException("undefined primary value: " + primary->value.lexeme);
        }
    }

    void visit_ternary(Ternary *ternary)
    {
        ternary->condition->accept(this);
        auto condition = this->stack.pop();

        ternary->true_expr->accept(this);
        auto true_expr = this->stack.pop();

        ternary->false_expr->accept(this);
        auto false_expr = this->stack.pop();

        BinaryenExpressionRef binaryen_ternary = BinaryenSelect(
            this->mod,
            condition.expression,
            true_expr.expression,
            false_expr.expression,
            BinaryenExpressionGetType(true_expr.expression)
        );

        this->stack.push(CodeGenExpression(binaryen_ternary));
    }

    void visit_const_stmt(ConstStmt *const_stmt)
    {
        const_stmt->value->accept(this);
        CodeGenExpression initializer_value = this->stack.pop();

        BinaryenType binaryen_type = const_stmt->type_identifier.has_value()
                                ? bird_type_to_binaryen_type(const_stmt->type_identifier.value())
                                : BinaryenExpressionGetType(initializer_value.expression);

        BinaryenIndex index = this->function_locals[this->current_function_name].size();
        this->function_locals[this->current_function_name].push_back(CodeGenBinaryenType(binaryen_type, initializer_value.type));

        BinaryenExpressionRef set_local = BinaryenLocalSet(this->mod, index, initializer_value.expression);
        environment.declare(const_stmt->identifier.lexeme, CodeGenIndex(index, initializer_value.type));

        this->stack.push(CodeGenExpression(set_local));
    }

    void visit_func(Func *func)
    {
        auto func_name = func->identifier.lexeme;

        if (func->return_type.has_value())
        {
            this->function_return_types[func_name] = CodeGenBinaryenType(
                bird_type_to_binaryen_type(func->return_type.value()),
                bird_type_to_code_gen_type(func->return_type.value())
            );
        }
        else
        {
            this->function_return_types[func_name] = CodeGenBinaryenType(BinaryenTypeNone());
        }

        auto old_function_name = this->current_function_name;

        this->current_function_name = func_name;
        auto current_function_body = std::vector<BinaryenExpressionRef>();
        this->function_locals[func_name] = std::vector<CodeGenBinaryenType>();

        std::vector<BinaryenType> param_types;

        for (auto &param : func->param_list)
        {
            param_types.push_back(bird_type_to_binaryen_type(param.second));
            this->function_locals[func_name].push_back(CodeGenBinaryenType(bird_type_to_binaryen_type(param.second)));
        }

        BinaryenType params = BinaryenTypeCreate(param_types.data(), param_types.size());

        BinaryenType result_type = func->return_type.has_value()
                                       ? bird_type_to_binaryen_type(func->return_type.value())
                                       : BinaryenTypeNone();

        this->environment.push_env();

        auto index = 0;
        for (auto &param : func->param_list)
        {
            this->environment.declare(param.first.lexeme, CodeGenIndex(index));
            current_function_body.push_back(
                BinaryenLocalGet(
                    this->mod,
                    index++,
                    bird_type_to_binaryen_type(param.second)));
        }

        auto found_return = false;
        for (auto &stmt : dynamic_cast<Block *>(func->block.get())->stmts)
        {
            try
            {
                stmt->accept(this);
            }
            catch (ReturnException _)
            {
                found_return = true;
            }
            auto result = this->stack.pop();

            current_function_body.push_back(result.expression);
        }

        if (!found_return)
        {
            current_function_body.push_back(
                BinaryenReturn(
                    this->mod,
                    nullptr));
        }

        this->environment.pop_env();

        BinaryenExpressionRef body = BinaryenBlock(
            this->mod,
            nullptr,
            current_function_body.data(),
            current_function_body.size(),
            BinaryenTypeNone());

        std::vector<CodeGenBinaryenType> vars = std::vector<CodeGenBinaryenType>(this->function_locals[func_name].begin() + param_types.size(), this->function_locals[func_name].end());

        std::vector<BinaryenType> binaryen_type_vars;
        for (CodeGenBinaryenType var : vars) {
            binaryen_type_vars.push_back(var.binaryen_type);
        }

        BinaryenFunctionRef func_type = BinaryenAddFunction(
            this->mod,
            func_name.c_str(),
            params,
            result_type,
            binaryen_type_vars.data(),
            binaryen_type_vars.size(),
            body);

        BinaryenAddFunctionExport(
            this->mod,
            func_name.c_str(),
            func_name.c_str());

        this->current_function_name = old_function_name;
        this->function_locals.erase(func_name);

        // no stack push here, automatically added
    }

    void visit_if_stmt(IfStmt *if_stmt)
    {
        if_stmt->condition->accept(this);
        auto condition = this->stack.pop();

        if_stmt->then_branch->accept(this);
        auto then_branch = this->stack.pop();

        if (if_stmt->else_branch.has_value())
        {
            if_stmt->else_branch.value()->accept(this);
            auto else_branch = this->stack.pop();

            this->stack.push(
                CodeGenExpression(BinaryenIf(
                    this->mod,
                    condition.expression,
                    then_branch.expression,
                    else_branch.expression)));
        }
        else
        {
            this->stack.push(
                CodeGenExpression(BinaryenIf(
                    this->mod,
                    condition.expression,
                    then_branch.expression,
                    nullptr)));
        }
    }

    void visit_call(Call *call)
    {
        auto func_name = call->identifier.lexeme;

        std::vector<BinaryenExpressionRef> args;

        for (auto &arg : call->args)
        {
            arg->accept(this);
            args.push_back(this->stack.pop().expression);
        }

        this->stack.push(CodeGenExpression(BinaryenCall(
            this->mod,
            func_name.c_str(),
            args.data(),
            args.size(),
            this->function_return_types[func_name].binaryen_type)));
    }

    void visit_return_stmt(ReturnStmt *return_stmt)
    {
        if (return_stmt->expr.has_value())
        {
            return_stmt->expr.value()->accept(this);
            this->stack.push(
                CodeGenExpression(BinaryenReturn(
                    this->mod,
                    this->stack.pop().expression)));
        }
        else
        {
            this->stack.push(
                CodeGenExpression(BinaryenReturn(
                    this->mod,
                    nullptr)));
        }

        throw ReturnException();
    }

    void visit_break_stmt(BreakStmt *break_stmt)
    {
    }

    void visit_continue_stmt(ContinueStmt *continue_stmt)
    {
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