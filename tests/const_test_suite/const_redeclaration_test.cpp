#include <gtest/gtest.h>
#include <vector>
#include <memory>
#include "../../include/visitors/interpreter.h"
#include "../../src/callable.cpp"
#include "../helpers/parse_test_helper.hpp"

TEST(VarTest, VarRedeclaration)
{
    auto code = "const x = 0;"
                "const x = 1;";
    auto ast = parse_code(code);

    Interpreter interpreter;
    ASSERT_THROW(interpreter.evaluate(&ast), BirdException);
}
