#include <gtest/gtest.h>
#include <vector>
#include <memory>
#include "../../include/exceptions/user_error_tracker.h"
#include "../../include/value.h"
#include "../../include/visitors/interpreter.h"
#include "../../src/callable.cpp"
#include "../helpers/parse_test_helper.hpp"

TEST(SymbolTableTest, ConstScope)
{
    auto code = "const x = 0;"
                "{"
                "x = 1;"
                "}";
    auto ast = parse_code(code);

    Interpreter interpreter;
    ASSERT_THROW(interpreter.evaluate(&ast), BirdException);
}
