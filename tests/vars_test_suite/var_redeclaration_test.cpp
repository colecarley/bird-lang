#include <gtest/gtest.h>
#include <vector>
#include <memory>
#include "../../include/exceptions/user_error_tracker.h"
#include "../../include/value.h"
#include "../../include/visitors/interpreter.h"
#include "../../src/callable.cpp"
#include "../helpers/parse_test_helper.hpp"

TEST(VarTest, VarRedeclaration)
{
    auto code = "var x = 0;"
                "var x = 1;";
    auto ast = parse_code(code);

    Interpreter interpreter;
    ASSERT_THROW(interpreter.evaluate(&ast), BirdException);
}
