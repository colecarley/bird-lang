#include <gtest/gtest.h>
#include <vector>
#include <memory>
#include "exceptions/user_error_tracker.h"
#include "value.h"
#include "visitors/interpreter.h"
#include "../src/callable.cpp"
#include "helpers/parse_test_helper.hpp"
#include "visitors/semantic_analyzer.h"
#include "visitors/type_checker.h"

TEST(SymbolTableTest, VarRedeclarationScope)
{
    auto code = "var x = 0;"
                "{"
                "var x = 1;"
                "}";
    auto ast = parse_code(code);

    auto user_error_tracker = UserErrorTracker(code);
    TypeChecker type_checker(&user_error_tracker);
    type_checker.check_types(&ast);
    ASSERT_FALSE(user_error_tracker.has_errors());

    Interpreter interpreter;
    interpreter.evaluate(&ast);

    ASSERT_TRUE(interpreter.env.contains("x"));
    ASSERT_TRUE(is_type<int>(interpreter.env.get("x")));
    ASSERT_EQ(as_type<int>(interpreter.env.get("x")), 0);
}