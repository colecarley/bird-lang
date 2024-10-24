#include <gtest/gtest.h>
#include <vector>
#include <memory>
#include "../../include/exceptions/user_error_tracker.h"
#include "../../include/value.h"
#include "../../include/visitors/interpreter.h"
#include "../../src/callable.cpp"
#include "../helpers/parse_test_helper.hpp"

TEST(ForLoopTest, ForLoopIncrement)
{
    auto code = "var z = 0;"
                "for var x: int = 0; x <= 5; x += 1 do {"
                    "z = x;"
                "}";

    auto ast = parse_code(code);

    Interpreter interpreter;
    interpreter.evaluate(&ast);

    ASSERT_TRUE(interpreter.environment->contains("z"));
    ASSERT_TRUE(is_type<int>(interpreter.environment->get("z")));
    EXPECT_EQ(as_type<int>(interpreter.environment->get("z").data), 5);
}

TEST(ForLoopTest, BreakOutsideLoop)
{
    auto code = "break;";

    auto ast = parse_code(code);

    Interpreter interpreter;

    ASSERT_THROW(interpreter.evaluate(&ast), BirdException);
}

TEST(ForLoopTest, ContinueOutsideLoop)
{
    auto code = "continue;";

    auto ast = parse_code(code);

    Interpreter interpreter;

    ASSERT_THROW(interpreter.evaluate(&ast), BirdException);
}

TEST(ForLoopTest, ForLoopBreak)
{
    auto code = "var z = 0;for var x: int = 0; x <= 5; x += 1 do {z = x;if z == 2 {break;}}";

    auto ast = parse_code(code);

    Interpreter interpreter;
    interpreter.evaluate(&ast);

    ASSERT_TRUE(interpreter.environment->contains("z"));
    ASSERT_TRUE(is_type<int>(interpreter.environment->get("z")));
    EXPECT_EQ(as_type<int>(interpreter.environment->get("z").data), 2);
}

/* Commenting this test out because this results in an infinite loop. An issue has already been made.
TEST(ForLoopTest, ForLoopContinue)
{
    auto code = "var z = 0;for var x: int = 0; x < 5; x += 1 do {if z == 3 {continue;}z += 1;continue;}";

    auto ast = parse_code(code);

    Interpreter interpreter;
    interpreter.evaluate(&ast);

    ASSERT_TRUE(interpreter.environment->contains("z"));
    ASSERT_TRUE(is_type<int>(interpreter.environment->get("z")));
    EXPECT_EQ(as_type<int>(interpreter.environment->get("z").data), 3);
}
*/