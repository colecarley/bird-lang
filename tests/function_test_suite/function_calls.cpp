#include <gtest/gtest.h>
#include <vector>
#include <memory>
#include "../../include/exceptions/user_error_tracker.h"
#include "../../include/value.h"
#include "../../include/visitors/interpreter.h"
#include "../../src/callable.cpp"
#include "../helpers/parse_test_helper.hpp"

TEST(FunctionTest, GoodFunctionCall)
{
    auto code = "fn function(i: int) -> int {return i;} "
                "var result: int = function(4);";
    auto ast = parse_code(code);

    Interpreter interpreter;
    interpreter.evaluate(&ast);

    ASSERT_TRUE(interpreter.call_table->contains("function"));
    ASSERT_TRUE(interpreter.environment->contains("resutl"));
    auto result = interpreter.environment->get("result");
    ASSERT_TRUE(is_type<int>(result));
    EXPECT_EQ(as_type<int>(result), 4);
}

TEST(FunctionTest, MalformedCall)
{
    auto code = "fn function() {} "
                "function(;";
    ASSERT_THROW(parse_code(code), UserException);
}

TEST(FunctionTest, CallWithIncorrectTypes)
{
    auto code = "fn function(i: int, j: string) {}"
                "function(4, 6);";
    auto ast = parse_code(code);

    Interpreter interpreter;
    ASSERT_THROW(interpreter.evaluate(&ast), BirdException);
}

TEST(FunctionTest, StoreReturnWithIncorrectVarType)
{
    auto code = "fn function() -> int {return 3;} "
                "var:string = function();";
    auto ast = parse_code(code);

    Interpreter interpreter;
    ASSERT_THROW(interpreter.evaluate(&ast), BirdException);
}