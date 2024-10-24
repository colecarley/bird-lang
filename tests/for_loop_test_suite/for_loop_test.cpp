#include <gtest/gtest.h>
#include <vector>
#include <memory>
#include "../../include/exceptions/user_error_tracker.h"
#include "../../include/value.h"
#include "../../include/visitors/interpreter.h"
#include "../../src/callable.cpp"
#include "../helpers/parse_test_helper.hpp"
#include "../../include/visitors/type_checker.h"

TEST(ForLoopTest, ForLoopIncrement)
{
    auto code = "var z = 0;"
                "for var x: int = 0; x <= 5; x += 1 do {"
                "z = x;"
                "}";

    auto ast = parse_code(code);

    auto user_error_tracker = UserErrorTracker(code);
    TypeChecker type_checker(&user_error_tracker);
    type_checker.check_types(&ast);
    ASSERT_FALSE(user_error_tracker.has_errors());

    Interpreter interpreter;
    interpreter.evaluate(&ast);

    ASSERT_TRUE(interpreter.environment->contains("z"));
    ASSERT_TRUE(is_type<int>(interpreter.environment->get("z")));
    EXPECT_EQ(as_type<int>(interpreter.environment->get("z").data), 5);
}