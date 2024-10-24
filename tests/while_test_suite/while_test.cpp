#include <gtest/gtest.h>
#include "../../include/value.h"
#include "../../include/visitors/interpreter.h"
#include "../../src/callable.cpp"
#include "../helpers/parse_test_helper.hpp"

TEST(WhileTest, While)
{
    auto code = "var x = 0; while x < 10 { x += 1; }";
    auto ast = parse_code(code);

    Interpreter interpreter;
    interpreter.evaluate(&ast);

    ASSERT_TRUE(interpreter.environment->contains("x"));
    ASSERT_TRUE(is_type<int>(interpreter.environment->get("x")));
    ASSERT_EQ(as_type<int>(interpreter.environment->get("x")), 10);
}

TEST(WhileTest, WhileFalse)
{
    auto code = "var x = 0; while false { x = 1; }";
    auto ast = parse_code(code);

    Interpreter interpreter;
    interpreter.evaluate(&ast);

    ASSERT_TRUE(interpreter.environment->contains("x"));
    ASSERT_TRUE(is_type<int>(interpreter.environment->get("x")));
    ASSERT_EQ(as_type<int>(interpreter.environment->get("x")), 0);
}

TEST(WhileTest, WhileBreak)
{
    auto code = "var x = 1; while true { x = 2; break; }";
    auto ast = parse_code(code);

    Interpreter interpreter;
    interpreter.evaluate(&ast);

    ASSERT_TRUE(interpreter.environment->contains("x"));
    ASSERT_TRUE(is_type<int>(interpreter.environment->get("x")));
    ASSERT_EQ(as_type<int>(interpreter.environment->get("x")), 2);
}

TEST(WhileTest, WhileContinue)
{
    auto code = "var x = 0; while true { x += 1; if (x <= 2) { continue; } break; }";
    auto ast = parse_code(code);

    Interpreter interpreter;
    interpreter.evaluate(&ast);

    ASSERT_TRUE(interpreter.environment->contains("x"));
    ASSERT_TRUE(is_type<int>(interpreter.environment->get("x")));
    ASSERT_EQ(as_type<int>(interpreter.environment->get("x")), 3);
}

TEST(WhileTest, WhileConstInc)
{
    auto code = "const inc = 1; var x = 0; while x < 10 { x += inc; } print x;";
    auto ast = parse_code(code);

    Interpreter interpreter;
    interpreter.evaluate(&ast);

    ASSERT_TRUE(interpreter.environment->contains("x"));
    ASSERT_TRUE(is_type<int>(interpreter.environment->get("x")));
    ASSERT_EQ(as_type<int>(interpreter.environment->get("x")), 10);

    ASSERT_TRUE(interpreter.environment->contains("inc"));
    ASSERT_TRUE(is_type<int>(interpreter.environment->get("inc")));
    ASSERT_EQ(as_type<int>(interpreter.environment->get("inc")), 1);
}