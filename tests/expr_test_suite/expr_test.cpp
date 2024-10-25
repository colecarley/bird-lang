#include <gtest/gtest.h>
#include <vector>
#include <memory>
#include "../../include/exceptions/user_error_tracker.h"
#include "../../include/value.h"
#include "../../include/visitors/interpreter.h"
#include "../../src/callable.cpp"
#include "../helpers/parse_test_helper.hpp"
#include "../../include/visitors/type_checker.h"

// Add modulus operator test when it has been implemented.
TEST(ExprTest, BinaryExpr)
{
    auto code = "var x = (10 + 1) * 3 / -3 - -3;";
    auto ast = parse_code(code);

    auto user_error_tracker = UserErrorTracker(code);
    TypeChecker type_checker(&user_error_tracker);
    type_checker.check_types(&ast);
    ASSERT_FALSE(user_error_tracker.has_errors());

    Interpreter interpreter;
    interpreter.evaluate(&ast);

    ASSERT_TRUE(interpreter.environment->contains("x"));
    ASSERT_TRUE(is_type<int>(interpreter.environment->get("x")));
    ASSERT_EQ(as_type<int>(interpreter.environment->get("x")), -8);
}

TEST(ExprTest, BinaryStringString)
{
    auto code = "var x = \"hello\" + \"there\";";
    auto ast = parse_code(code);

    auto user_error_tracker = UserErrorTracker(code);
    TypeChecker type_checker(&user_error_tracker);
    type_checker.check_types(&ast);
    ASSERT_FALSE(user_error_tracker.has_errors());

    Interpreter interpreter;
    interpreter.evaluate(&ast);

    ASSERT_TRUE(interpreter.environment->contains("x"));
    ASSERT_TRUE(is_type<std::string>(interpreter.environment->get("x")));
    ASSERT_EQ(as_type<std::string>(interpreter.environment->get("x")), "hellothere");
}

TEST(ExprTest, BinaryIntString)
{
    auto code = "var x = 1 + \"test\";";
    auto ast = parse_code(code);

    auto user_error_tracker = UserErrorTracker(code);
    TypeChecker type_checker(&user_error_tracker);
    type_checker.check_types(&ast);
    ASSERT_FALSE(user_error_tracker.has_errors());

    Interpreter interpreter;

    ASSERT_THROW(interpreter.evaluate(&ast), BirdException);
}

TEST(ExprTest, BinaryFloatString)
{
    auto code = "var x = 1.1 + \"test\";";
    auto ast = parse_code(code);

    auto user_error_tracker = UserErrorTracker(code);
    TypeChecker type_checker(&user_error_tracker);
    type_checker.check_types(&ast);
    ASSERT_FALSE(user_error_tracker.has_errors());

    Interpreter interpreter;

    ASSERT_THROW(interpreter.evaluate(&ast), BirdException);
}

TEST(ExprTest, BinaryBoolInt)
{
    auto code = "var x = true + 11;";
    auto ast = parse_code(code);

    auto user_error_tracker = UserErrorTracker(code);
    TypeChecker type_checker(&user_error_tracker);
    type_checker.check_types(&ast);
    ASSERT_FALSE(user_error_tracker.has_errors());

    Interpreter interpreter;

    ASSERT_THROW(interpreter.evaluate(&ast), BirdException);
}

TEST(ExprTest, CondExpr)
{
    auto code = "var x = 1 == 1 != false;";
    auto ast = parse_code(code);

    auto user_error_tracker = UserErrorTracker(code);
    TypeChecker type_checker(&user_error_tracker);
    type_checker.check_types(&ast);
    ASSERT_FALSE(user_error_tracker.has_errors());

    Interpreter interpreter;
    interpreter.evaluate(&ast);

    ASSERT_TRUE(interpreter.environment->contains("x"));
    ASSERT_TRUE(is_type<bool>(interpreter.environment->get("x")));
    ASSERT_EQ(as_type<bool>(interpreter.environment->get("x")), true);
}

TEST(ExprTest, CondExprIntInt)
{
    auto code = "var x = 1 >= 1;";
    auto ast = parse_code(code);

    auto user_error_tracker = UserErrorTracker(code);
    TypeChecker type_checker(&user_error_tracker);
    type_checker.check_types(&ast);
    ASSERT_FALSE(user_error_tracker.has_errors());

    Interpreter interpreter;
    interpreter.evaluate(&ast);

    ASSERT_TRUE(interpreter.environment->contains("x"));
    ASSERT_TRUE(is_type<bool>(interpreter.environment->get("x")));
    ASSERT_EQ(as_type<bool>(interpreter.environment->get("x")), true);
}

TEST(ExprTest, CondExprFloatIntOverflow)
{
    auto code = "var x = 0.9999999999 < 1;";
    auto ast = parse_code(code);

    auto user_error_tracker = UserErrorTracker(code);
    TypeChecker type_checker(&user_error_tracker);
    type_checker.check_types(&ast);
    ASSERT_FALSE(user_error_tracker.has_errors());

    Interpreter interpreter;
    interpreter.evaluate(&ast);

    ASSERT_TRUE(interpreter.environment->contains("x"));
    ASSERT_TRUE(is_type<bool>(interpreter.environment->get("x")));
    ASSERT_EQ(as_type<bool>(interpreter.environment->get("x")), true);
}

TEST(ExprTest, CondExprIntFloatOverflow)
{
    auto code = "var x = 1 > 0.9999999999;";
    auto ast = parse_code(code);

    auto user_error_tracker = UserErrorTracker(code);
    TypeChecker type_checker(&user_error_tracker);
    type_checker.check_types(&ast);
    ASSERT_FALSE(user_error_tracker.has_errors());

    Interpreter interpreter;
    interpreter.evaluate(&ast);

    ASSERT_TRUE(interpreter.environment->contains("x"));
    ASSERT_TRUE(is_type<bool>(interpreter.environment->get("x")));
    ASSERT_EQ(as_type<bool>(interpreter.environment->get("x")), true);
}

TEST(ExprTest, CondExprIntFloat)
{
    auto code = "var x = 10532 == 10532.0;";
    auto ast = parse_code(code);

    auto user_error_tracker = UserErrorTracker(code);
    TypeChecker type_checker(&user_error_tracker);
    type_checker.check_types(&ast);
    ASSERT_FALSE(user_error_tracker.has_errors());

    Interpreter interpreter;
    interpreter.evaluate(&ast);

    ASSERT_TRUE(interpreter.environment->contains("x"));
    ASSERT_TRUE(is_type<bool>(interpreter.environment->get("x")));
    ASSERT_EQ(as_type<bool>(interpreter.environment->get("x")), true);
}

TEST(ExprTest, CondExprBoolString)
{
    auto code = "var x = true < \"true\";";
    auto ast = parse_code(code);

    auto user_error_tracker = UserErrorTracker(code);
    TypeChecker type_checker(&user_error_tracker);
    type_checker.check_types(&ast);
    ASSERT_FALSE(user_error_tracker.has_errors());

    Interpreter interpreter;

    ASSERT_THROW(interpreter.evaluate(&ast), BirdException);
}

TEST(ExprTest, CondExprFloatBool)
{
    auto code = "var x = 1.1 >= \"true\";";
    auto ast = parse_code(code);

    auto user_error_tracker = UserErrorTracker(code);
    TypeChecker type_checker(&user_error_tracker);
    type_checker.check_types(&ast);
    ASSERT_FALSE(user_error_tracker.has_errors());

    Interpreter interpreter;

    ASSERT_THROW(interpreter.evaluate(&ast), BirdException);
}

TEST(ExprTest, IdentifierInExpr)
{
    auto code = "var z: int = 7; const y: float = -9.2; var x = 1 - (z * y) - -y;";
    auto ast = parse_code(code);

    auto user_error_tracker = UserErrorTracker(code);
    TypeChecker type_checker(&user_error_tracker);
    type_checker.check_types(&ast);
    ASSERT_FALSE(user_error_tracker.has_errors());

    Interpreter interpreter;
    interpreter.evaluate(&ast);

    ASSERT_TRUE(interpreter.environment->contains("x"));
    ASSERT_TRUE(is_type<double>(interpreter.environment->get("x")));
    ASSERT_EQ(as_type<double>(interpreter.environment->get("x")), 56.2f);
}

TEST(ExprTest, BinaryDivideByZero)
{
    auto code = "var x = 10 / 0;";
    auto ast = parse_code(code);

    auto user_error_tracker = UserErrorTracker(code);
    TypeChecker type_checker(&user_error_tracker);
    type_checker.check_types(&ast);
    ASSERT_FALSE(user_error_tracker.has_errors());

    Interpreter interpreter;

    ASSERT_THROW(interpreter.evaluate(&ast), BirdException);
}