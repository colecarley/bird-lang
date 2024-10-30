#include <gtest/gtest.h>
#include <vector>
#include <memory>
#include "../../include/exceptions/user_error_tracker.h"
#include "../../include/value.h"
#include "../../include/visitors/interpreter.h"
#include "../../src/callable.cpp"
#include "../helpers/parse_test_helper.hpp"
#include "../../include/visitors/type_checker.h"

// INT
TEST(VarTest, VarWithTypeInt)
{
    auto code = "var x: int = 4;";
    auto ast = parse_code(code);

    auto user_error_tracker = UserErrorTracker(code);
    TypeChecker type_checker(&user_error_tracker);
    type_checker.check_types(&ast);
    ASSERT_FALSE(user_error_tracker.has_errors());

    Interpreter interpreter;
    interpreter.evaluate(&ast);

    ASSERT_TRUE(interpreter.environment->contains("x"));
    ASSERT_TRUE(is_type<int>(interpreter.environment->get("x")));
    ASSERT_EQ(as_type<int>(interpreter.environment->get("x")), 4);
}

// FLOAT
TEST(VarTest, VarWithTypeFloat)
{
    auto code = "var x: float = 4.0;";
    auto ast = parse_code(code);

    auto user_error_tracker = UserErrorTracker(code);
    TypeChecker type_checker(&user_error_tracker);
    type_checker.check_types(&ast);
    ASSERT_FALSE(user_error_tracker.has_errors());

    Interpreter interpreter;
    interpreter.evaluate(&ast);

    ASSERT_TRUE(interpreter.environment->contains("x"));
    ASSERT_TRUE(is_type<double>(interpreter.environment->get("x")));
    ASSERT_EQ(as_type<double>(interpreter.environment->get("x")), 4.0);

    code = "var y: float = 4;";
    ast = parse_code(code);

    type_checker.check_types(&ast);
    ASSERT_FALSE(user_error_tracker.has_errors());

    interpreter.evaluate(&ast);

    ASSERT_TRUE(interpreter.environment->contains("y"));
    ASSERT_TRUE(is_type<double>(interpreter.environment->get("y")));
    ASSERT_EQ(as_type<double>(interpreter.environment->get("y")), 4.0);
}

// STRINGS
TEST(VarTest, VarWithTypeString)
{
    auto code = "var x: str = \"hello\";";
    auto ast = parse_code(code);

    auto user_error_tracker = UserErrorTracker(code);
    TypeChecker type_checker(&user_error_tracker);
    type_checker.check_types(&ast);
    ASSERT_FALSE(user_error_tracker.has_errors());

    Interpreter interpreter;
    interpreter.evaluate(&ast);

    ASSERT_TRUE(interpreter.environment->contains("x"));
    ASSERT_TRUE(is_type<std::string>(interpreter.environment->get("x")));
    ASSERT_EQ(as_type<std::string>(interpreter.environment->get("x")), "hello");
}

// BOOL
TEST(VarTest, VarWithTypeBool)
{
    auto code = "var x: bool = true;";
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

    code = "var y: bool = false;";
    ast = parse_code(code);

    type_checker.check_types(&ast);
    ASSERT_FALSE(user_error_tracker.has_errors());

    interpreter.evaluate(&ast);

    ASSERT_TRUE(interpreter.environment->contains("y"));
    ASSERT_TRUE(is_type<bool>(interpreter.environment->get("y")));
    ASSERT_EQ(as_type<bool>(interpreter.environment->get("y")), false);
}
