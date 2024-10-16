#include <gtest/gtest.h>
// #include "../src/lexer.cpp"
// #include "../src/parser.cpp"
#include "../include/exceptions/user_error_tracker.h"
#include "../helpers/parse_test_helper.hpp"

TEST(ParserTest, ParseBlockStmt)
{
    std::string code = "{ var x: int = 10; var y: int = 20; }";

    auto stmts = parse_code(code);

    ASSERT_EQ(stmts.size(), 1); // should parse to 1 block statement

    Block *block = dynamic_cast<Block *>(stmts[0].get()); // verify statement is of type Block
    ASSERT_NE(block, nullptr);                            // if NE will fail since dynamic_cast returned nullptr

    // make sure the block contains 2 statements
    ASSERT_EQ(block->stmts.size(), 2);

    // verify first statement is of type DeclStmt
    DeclStmt *first_decl_stmt = dynamic_cast<DeclStmt *>(block->stmts[0].get());
    ASSERT_NE(first_decl_stmt, nullptr);
    EXPECT_EQ(first_decl_stmt->identifier.lexeme, "x"); // ensure lexeme is correct for identifier
    EXPECT_EQ(first_decl_stmt->type_identifier.has_value(), true);
    EXPECT_EQ(first_decl_stmt->type_identifier.value().lexeme, "int"); // ensure lexeme is correct for type ID

    // ensure primary expression is set
    Primary *first_decl_value = dynamic_cast<Primary *>(first_decl_stmt->value.get());
    ASSERT_NE(first_decl_value, nullptr);
    EXPECT_EQ(first_decl_value->value.lexeme, "10");

    // repeat
    DeclStmt *second_decl_stmt = dynamic_cast<DeclStmt *>(block->stmts[1].get());
    ASSERT_NE(second_decl_stmt, nullptr);
    EXPECT_EQ(second_decl_stmt->identifier.lexeme, "y");
    EXPECT_EQ(second_decl_stmt->type_identifier.has_value(), true);
    EXPECT_EQ(second_decl_stmt->type_identifier.value().lexeme, "int");

    Primary *second_decl_value = dynamic_cast<Primary *>(second_decl_stmt->value.get());
    ASSERT_NE(second_decl_value, nullptr);
    EXPECT_EQ(second_decl_value->value.lexeme, "20");
}

TEST(ParserTest, ParseConstStmt)
{
    std::string code = "const z: str = \"foobar\";";

    auto stmts = parse_code(code);

    ASSERT_EQ(stmts.size(), 1);

    ConstStmt *const_stmt = dynamic_cast<ConstStmt *>(stmts[0].get());
    ASSERT_NE(const_stmt, nullptr);
    EXPECT_EQ(const_stmt->identifier.lexeme, "z");
    EXPECT_EQ(const_stmt->type_identifier.has_value(), true);
    EXPECT_EQ(const_stmt->type_identifier.value().lexeme, "str");

    Primary *primary_expr = dynamic_cast<Primary *>(const_stmt->value.get());
    ASSERT_NE(primary_expr, nullptr);
    EXPECT_EQ(primary_expr->value.lexeme, "foobar");
}

TEST(ParserTest, ParseDeclStmt)
{
    std::string code = "var x: int = 3;";

    auto stmts = parse_code(code);

    ASSERT_EQ(stmts.size(), 1);

    DeclStmt *decl_stmt = dynamic_cast<DeclStmt *>(stmts[0].get());
    ASSERT_NE(decl_stmt, nullptr);
    EXPECT_EQ(decl_stmt->identifier.lexeme, "x");
    EXPECT_EQ(decl_stmt->type_identifier.has_value(), true);
    EXPECT_EQ(decl_stmt->type_identifier.value().lexeme, "int");

    Primary *primary_expr = dynamic_cast<Primary *>(decl_stmt->value.get());
    ASSERT_NE(primary_expr, nullptr);
    EXPECT_EQ(primary_expr->value.lexeme, "3");
}

TEST(ParserTest, ParseExprStmt)
{
    /*
     *                    (1 + 2 - 3 / 6 * 4 <=  4)
     *                        /              |    \
     *           (1 + 2 - 3 / 6 * 4)        `<=`  `4`
     *          /       |        \
     *     (1 + 2)     `-`    (3 / 6 * 4)
     *    /   |  \            /      |  \
     *  `1`  `+` `2`      (3  / 6)  `*` `4`
     *                     /  |   \
     *                   `3` `/` `6`
     */
    std::string code = "1 + 2 - 3 / 6 * 4 <= 4;";

    auto stmts = parse_code(code);

    ASSERT_EQ(stmts.size(), 1);
    ExprStmt *expr_stmt = dynamic_cast<ExprStmt *>(stmts[0].get());
    ASSERT_NE(expr_stmt, nullptr);

    // outer binary expression `1 + 2 - 3 / 6 * 4 <= 4`
    Binary *outer_binary_expr = dynamic_cast<Binary *>(expr_stmt->expr.get());
    ASSERT_NE(outer_binary_expr, nullptr);
    EXPECT_EQ(outer_binary_expr->op.lexeme, "<=");

    // rhs LESS_EQUAL ... <= `4`
    Primary *rhs_primary = dynamic_cast<Primary *>(outer_binary_expr->right.get());
    ASSERT_NE(rhs_primary, nullptr);
    EXPECT_EQ(rhs_primary->value.lexeme, "4");

    // lhs of LESS_EQUAL `1 + 2 - 3 / 6 * 4` <= ...
    Binary *lhs_binary_expr = dynamic_cast<Binary *>(outer_binary_expr->left.get());
    ASSERT_NE(lhs_binary_expr, nullptr);
    EXPECT_EQ(lhs_binary_expr->op.lexeme, "-");

    // lhs of MINUS `1 + 2` - ...
    Binary *addition_expr = dynamic_cast<Binary *>(lhs_binary_expr->left.get());
    ASSERT_NE(addition_expr, nullptr);
    EXPECT_EQ(addition_expr->op.lexeme, "+");

    // lhs of PLUS `1` + ...
    Primary *lhs_primary_one = dynamic_cast<Primary *>(addition_expr->left.get());
    ASSERT_NE(lhs_primary_one, nullptr);
    EXPECT_EQ(lhs_primary_one->value.lexeme, "1");

    // rhs of PLUS ... + `2`
    Primary *rhs_primary_two = dynamic_cast<Primary *>(addition_expr->right.get());
    ASSERT_NE(rhs_primary_two, nullptr);
    EXPECT_EQ(rhs_primary_two->value.lexeme, "2");

    // rhs of MINUS ... - `3 / 6 * 4`
    Binary *multiplication_expr = dynamic_cast<Binary *>(lhs_binary_expr->right.get());
    ASSERT_NE(multiplication_expr, nullptr);
    EXPECT_EQ(multiplication_expr->op.lexeme, "*");

    // lhs of STAR `3 / 6` * ...
    Binary *division_expr = dynamic_cast<Binary *>(multiplication_expr->left.get());
    ASSERT_NE(division_expr, nullptr);
    EXPECT_EQ(division_expr->op.lexeme, "/");

    // lhs of SLASH `3` / ...
    Primary *lhs_primary_three = dynamic_cast<Primary *>(division_expr->left.get());
    ASSERT_NE(lhs_primary_three, nullptr);
    EXPECT_EQ(lhs_primary_three->value.lexeme, "3");

    // rhs of SLASH ... / `6`
    Primary *rhs_primary_six = dynamic_cast<Primary *>(division_expr->right.get());
    ASSERT_NE(rhs_primary_six, nullptr);
    EXPECT_EQ(rhs_primary_six->value.lexeme, "6");

    // rhs of STAR ... * `4`
    Primary *rhs_primary_four = dynamic_cast<Primary *>(multiplication_expr->right.get());
    ASSERT_NE(rhs_primary_four, nullptr);
    EXPECT_EQ(rhs_primary_four->value.lexeme, "4");

    // std::cout << "expression: " << std::endl;
    // print_expr(expr_stmt->expr.get());
    // std::cout << std::endl;
}

TEST(ParserTest, ParseFuncStmt)
{
    /*
     * fn IDENTIFIER (IDENTIFIER: TYPE_IDENTIFIER, IDENTIFIER: TYPE_IDENTIFIER) ARROW TYPE_IDENTIFIER { BLOCK; }
     */
    std::string code = "fn add(first: int, second: int) -> int { first + second; }";

    auto stmts = parse_code(code);

    ASSERT_EQ(stmts.size(), 1);
    Func *func_stmt = dynamic_cast<Func *>(stmts[0].get());
    ASSERT_NE(func_stmt, nullptr);

    // match Func class identifier, check optional return type
    EXPECT_EQ(func_stmt->identifier.lexeme, "add");
    ASSERT_TRUE(func_stmt->return_type.has_value());
    EXPECT_EQ(func_stmt->return_type->lexeme, "int");

    // std::pair (first: IDENTIFIER, second: TYPE_IDENTIFIER)
    ASSERT_EQ(func_stmt->param_list.size(), 2);
    EXPECT_EQ(func_stmt->param_list[0].first.lexeme, "first");
    EXPECT_EQ(func_stmt->param_list[0].second.lexeme, "int");
    EXPECT_EQ(func_stmt->param_list[1].first.lexeme, "second");
    EXPECT_EQ(func_stmt->param_list[1].second.lexeme, "int");

    // get Block and check for not nullptr
    Block *block_stmt = dynamic_cast<Block *>(func_stmt->block.get());
    ASSERT_NE(block_stmt, nullptr);
    ASSERT_EQ(block_stmt->stmts.size(), 1);

    // get expression statement, check for not nullptr
    ExprStmt *expr_stmt = dynamic_cast<ExprStmt *>(block_stmt->stmts[0].get());
    ASSERT_NE(expr_stmt, nullptr);

    // get binary expression `first + second` check op
    Binary *binary_expr = dynamic_cast<Binary *>(expr_stmt->expr.get());
    ASSERT_NE(binary_expr, nullptr);
    EXPECT_EQ(binary_expr->op.lexeme, "+");

    // back to expression parse checking, luckily not as bad as last expression test
    Primary *lhs_primary = dynamic_cast<Primary *>(binary_expr->left.get());
    ASSERT_NE(lhs_primary, nullptr);
    EXPECT_EQ(lhs_primary->value.lexeme, "first");

    Primary *rhs_primary = dynamic_cast<Primary *>(binary_expr->right.get());
    ASSERT_NE(rhs_primary, nullptr);
    EXPECT_EQ(rhs_primary->value.lexeme, "second");
}

TEST(ParserTest, ParseFunctionNoArgsNoReturnType)
{
    std::string code = "fn function() {}";

    auto stmts = parse_code(code);

    ASSERT_EQ(stmts.size(), 1);

    Func *func_stmt = dynamic_cast<Func *>(stmts[0].get());
    ASSERT_NE(func_stmt, nullptr);

    EXPECT_EQ(func_stmt->identifier.lexeme, "function");
    ASSERT_FALSE(func_stmt->return_type.has_value());

    ASSERT_EQ(func_stmt->param_list.size(), 0);

    // get Block and check for not nullptr
    Block *block_stmt = dynamic_cast<Block *>(func_stmt->block.get());
    ASSERT_NE(block_stmt, nullptr);
    ASSERT_EQ(block_stmt->stmts.size(), 0);
}

TEST(ParserTest, ParseFunctionNoArgs)
{
    std::string code = "fn function() -> int {}";

    auto stmts = parse_code(code);

    ASSERT_EQ(stmts.size(), 1);

    Func *func_stmt = dynamic_cast<Func *>(stmts[0].get());
    ASSERT_NE(func_stmt, nullptr);

    EXPECT_EQ(func_stmt->identifier.lexeme, "function");
    ASSERT_TRUE(func_stmt->return_type.has_value());
    EXPECT_EQ(func_stmt->return_type->lexeme, "int");

    ASSERT_EQ(func_stmt->param_list.size(), 0);

    // get Block and check for not nullptr
    Block *block_stmt = dynamic_cast<Block *>(func_stmt->block.get());
    ASSERT_NE(block_stmt, nullptr);
    ASSERT_EQ(block_stmt->stmts.size(), 0);
}

TEST(ParserTest, ParseFunctionNoReturnType)
{
    std::string code = "fn function(i: int, j: string) {}";

    auto stmts = parse_code(code);

    ASSERT_EQ(stmts.size(), 1);

    Func *func_stmt = dynamic_cast<Func *>(stmts[0].get());
    ASSERT_NE(func_stmt, nullptr);

    EXPECT_EQ(func_stmt->identifier.lexeme, "function");
    ASSERT_FALSE(func_stmt->return_type.has_value());

    ASSERT_EQ(func_stmt->param_list.size(), 2);
    EXPECT_EQ(func_stmt->param_list[0].first.lexeme, "i");
    EXPECT_EQ(func_stmt->param_list[0].second.lexeme, "int");
    EXPECT_EQ(func_stmt->param_list[1].first.lexeme, "j");
    EXPECT_EQ(func_stmt->param_list[1].second.lexeme, "string");

    // get Block and check for not nullptr
    Block *block_stmt = dynamic_cast<Block *>(func_stmt->block.get());
    ASSERT_NE(block_stmt, nullptr);
    ASSERT_EQ(block_stmt->stmts.size(), 0);
}

TEST(ParserTest, FunctionFailsArrowNoReturnType)
{
    std::string code = "fn function() -> {}";

    UserErrorTracker error_tracker(code);

    ASSERT_THROW(auto stmts = parse_code_with_error_tracker(code, error_tracker), UserException);

    ASSERT_TRUE(error_tracker.has_errors());

    auto errors = error_tracker.get_errors();

    ASSERT_EQ(errors.size(), 1);
    EXPECT_EQ(errors[0], "expected return type after arrow operator");
}

TEST(ParserTest, ParseIfStmt)
{
    /*
     * if EXPRESSION STATEMENT;
     * else if EXPRESSION STATEMENT;
     * else STATEMENT;
     */
    std::string code = "if 1 > 2 print 1; else if 1 == 2 print 2; else print 3;";

    auto stmts = parse_code(code);

    ASSERT_EQ(stmts.size(), 1);
    IfStmt *if_stmt = dynamic_cast<IfStmt *>(stmts[0].get());
    ASSERT_NE(if_stmt, nullptr);

    // binary expression for conditional
    Binary *condition_if = dynamic_cast<Binary *>(if_stmt->condition.get());
    ASSERT_NE(condition_if, nullptr);
    EXPECT_EQ(condition_if->op.lexeme, ">");

    // left is primary should be `1`
    Primary *lhs_condition_if = dynamic_cast<Primary *>(condition_if->left.get());
    ASSERT_NE(lhs_condition_if, nullptr);
    EXPECT_EQ(lhs_condition_if->value.lexeme, "1");

    // right is primary should be `1`
    Primary *rhs_condition_if = dynamic_cast<Primary *>(condition_if->right.get());
    ASSERT_NE(rhs_condition_if, nullptr);
    EXPECT_EQ(rhs_condition_if->value.lexeme, "2");

    // then branch should have a print statement with one arg (primary expression)
    PrintStmt *then_print_stmt = dynamic_cast<PrintStmt *>(if_stmt->then_branch.get());
    ASSERT_NE(then_print_stmt, nullptr);
    ASSERT_EQ(then_print_stmt->args.size(), 1);

    // then arg is a primary expression should be `1`
    Primary *then_arg = dynamic_cast<Primary *>(then_print_stmt->args[0].get());
    ASSERT_NE(then_arg, nullptr);
    EXPECT_EQ(then_arg->value.lexeme, "1");

    /*
     * check if else branch has value, in this test
     * we ASSERT_TRUE because we already know it should
     */

    ASSERT_TRUE(if_stmt->else_branch.has_value());
    IfStmt *else_if_stmt = dynamic_cast<IfStmt *>(if_stmt->else_branch->get());
    ASSERT_NE(else_if_stmt, nullptr);

    // another binary expression as condition, expect `==`
    Binary *condition_else_if = dynamic_cast<Binary *>(else_if_stmt->condition.get());
    ASSERT_NE(condition_else_if, nullptr);
    EXPECT_EQ(condition_else_if->op.lexeme, "==");

    // lhs else if, primary expression = `1`
    Primary *lhs_condition_else_if = dynamic_cast<Primary *>(condition_else_if->left.get());
    ASSERT_NE(lhs_condition_else_if, nullptr);
    EXPECT_EQ(lhs_condition_else_if->value.lexeme, "1");

    // rhs else if, primary expression = `2`
    Primary *rhs_condition_else_if = dynamic_cast<Primary *>(condition_else_if->right.get());
    ASSERT_NE(rhs_condition_else_if, nullptr);
    EXPECT_EQ(rhs_condition_else_if->value.lexeme, "2");

    // repeat
    PrintStmt *else_if_print_stmt = dynamic_cast<PrintStmt *>(else_if_stmt->then_branch.get());
    ASSERT_NE(else_if_print_stmt, nullptr);
    ASSERT_EQ(else_if_print_stmt->args.size(), 1);

    Primary *else_if_arg = dynamic_cast<Primary *>(else_if_print_stmt->args[0].get());
    ASSERT_NE(else_if_arg, nullptr);
    EXPECT_EQ(else_if_arg->value.lexeme, "2");

    ASSERT_TRUE(else_if_stmt->else_branch.has_value());
    PrintStmt *else_print_stmt = dynamic_cast<PrintStmt *>(else_if_stmt->else_branch->get());
    ASSERT_NE(else_print_stmt, nullptr);
    ASSERT_EQ(else_print_stmt->args.size(), 1);

    Primary *else_arg = dynamic_cast<Primary *>(else_print_stmt->args[0].get());
    ASSERT_NE(else_arg, nullptr);
    EXPECT_EQ(else_arg->value.lexeme, "3");
}

TEST(ParserTest, ParsePrintStmt)
{
    /*
     * PRINT EXPRESSION[i]
     */
    std::string code = "print 28;";

    auto stmts = parse_code(code);

    ASSERT_EQ(stmts.size(), 1);
    PrintStmt *print_stmt = dynamic_cast<PrintStmt *>(stmts[0].get());
    ASSERT_NE(print_stmt, nullptr);

    // print primary expression `28`
    ASSERT_EQ(print_stmt->args.size(), 1);
    Primary *primary_expr = dynamic_cast<Primary *>(print_stmt->args[0].get());
    ASSERT_NE(primary_expr, nullptr);
    EXPECT_EQ(primary_expr->value.lexeme, "28");
}

TEST(ParserTest, ParseWhileStmt)
{
    std::string code = "while (true) { print 1; }";

    auto stmts = parse_code(code);

    // check for while statement
    ASSERT_EQ(stmts.size(), 1);
    WhileStmt *while_stmt = dynamic_cast<WhileStmt *>(stmts[0].get());
    ASSERT_NE(while_stmt, nullptr);

    // primary expression as condition: `true`
    Primary *condition_expr = dynamic_cast<Primary *>(while_stmt->condition.get());
    ASSERT_NE(condition_expr, nullptr);
    EXPECT_EQ(condition_expr->value.lexeme, "true");

    // check for block
    Block *block_stmt = dynamic_cast<Block *>(while_stmt->stmt.get());
    ASSERT_NE(block_stmt, nullptr);
    ASSERT_EQ(block_stmt->stmts.size(), 1);

    // check for print statement with one arg
    PrintStmt *print_stmt = dynamic_cast<PrintStmt *>(block_stmt->stmts[0].get());
    ASSERT_NE(print_stmt, nullptr);
    ASSERT_EQ(print_stmt->args.size(), 1);

    Primary *print_arg = dynamic_cast<Primary *>(print_stmt->args[0].get());
    ASSERT_NE(print_arg, nullptr);
    EXPECT_EQ(print_arg->value.lexeme, "1");
}

// Should be incorporated into a comprehensive loop test when loops are implemented.
TEST(ParserTest, ParseBreakStmt)
{
    std::string code = "break;";

    auto stmts = parse_code(code);

    // check for break statement
    ASSERT_EQ(stmts.size(), 1);
    BreakStmt *break_stmt = dynamic_cast<BreakStmt *>(stmts[0].get());
    ASSERT_NE(break_stmt, nullptr);
}

// Should be incorporated into a comprehensive loop test when loops are implemented.
TEST(ParserTest, ParseContinueStmt)
{
    std::string code = "continue;";

    auto stmts = parse_code(code);

    // check for break statement
    ASSERT_EQ(stmts.size(), 1);
    ContinueStmt *continue_stmt = dynamic_cast<ContinueStmt *>(stmts[0].get());
    ASSERT_NE(continue_stmt, nullptr);
}