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
    auto code = "for var x: int = 0; x <= 5; x += 1 do print x;";
    auto ast = parse_code(code);

    Interpreter interpreter;
    interpreter.evaluate(&ast);

    ASSERT_TRUE(interpreter.environment->contains("x"));
    ASSERT_TRUE(is_type<int>(interpreter.environment->get("x")));
    EXPECT_EQ(as_type<int>(interpreter.environment->get("x").data), 5);
}