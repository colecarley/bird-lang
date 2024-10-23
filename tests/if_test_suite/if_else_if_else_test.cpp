#include <gtest/gtest.h>
#include <vector>
#include <memory>
#include "../../include/exceptions/user_error_tracker.h"
#include "../../include/value.h"
#include "../../include/visitors/interpreter.h"
#include "../../src/callable.cpp"
#include "../helpers/parse_test_helper.hpp"

TEST(IfTest, IfElseTrue)
{
    auto code = "var x = 1;"
                "if false {"
                "x = 2;"
                "} else if true {"
                "x = 3;"
                "} else {"
                "x = 4;"
                "}";
    auto ast = parse_code(code);

    Interpreter interpreter;
    interpreter.evaluate(&ast);

    ASSERT_TRUE(interpreter.environment->contains("x"));
    ASSERT_TRUE(is_type<int>(interpreter.environment->get("x")));
    ASSERT_EQ(as_type<int>(interpreter.environment->get("x")), 3);
}

TEST(IfTest, IfElseFalse)
{
    auto code = "var x = 1;"
                "if false {"
                "x = 2;"
                "} else if false {"
                "x = 3;"
                "} else {"
                "x = 4;"
                "}";
    auto ast = parse_code(code);

    Interpreter interpreter;
    interpreter.evaluate(&ast);

    ASSERT_TRUE(interpreter.environment->contains("x"));
    ASSERT_TRUE(is_type<int>(interpreter.environment->get("x")));
    ASSERT_EQ(as_type<int>(interpreter.environment->get("x")), 4);
}