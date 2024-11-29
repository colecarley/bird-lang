#include <gtest/gtest.h>
#include <vector>
#include <memory>
#include "../../include/exceptions/user_error_tracker.h"
#include "../../include/value.h"
#include "../../include/visitors/interpreter.h"
#include "../../src/callable.cpp"
#include "../helpers/parse_test_helper.hpp"
#include "../../include/visitors/semantic_analyzer.h"
#include "../../include/visitors/type_checker.h"

TEST(FunctionTest, GoodFunctionCall)
{
    auto code = "fn function(i: int) -> int {return i;} "
                "var result: int = function(4);";
    auto ast = parse_code(code);

    auto user_error_tracker = UserErrorTracker(code);
    SemanticAnalyzer analyze_semantics(&user_error_tracker);
    analyze_semantics.analyze_semantics(&ast);
    ASSERT_FALSE(user_error_tracker.has_errors());

    TypeChecker type_checker(&user_error_tracker);
    type_checker.check_types(&ast);
    ASSERT_FALSE(user_error_tracker.has_errors());

    Interpreter interpreter;
    interpreter.evaluate(&ast);

    ASSERT_TRUE(interpreter.call_table.contains("function"));
    ASSERT_TRUE(interpreter.env.contains("result"));
    auto result = interpreter.env.get("result");
    ASSERT_TRUE(is_type<int>(result));
    EXPECT_EQ(as_type<int>(result), 4);
}

TEST(FunctionTest, MalformedCall)
{
    auto code = "fn function() {} "
                "function(;";

    auto user_error_tracker = UserErrorTracker(code);
    parse_code_with_error_tracker(code, user_error_tracker);

    ASSERT_TRUE(user_error_tracker.has_errors());
    auto errors = user_error_tracker.get_errors();
    ASSERT_EQ(errors.size(), 1);
    // TODO: fix double space
    EXPECT_EQ(std::get<0>(errors[0]), ">>[ERROR] expected identifier or i32  (line 0, character 26)");
}

TEST(FunctionTest, CallWithIncorrectTypes)
{
    auto code = "fn function(i: int, j: str) {}"
                "function(4, 6);";
    auto ast = parse_code(code);

    auto user_error_tracker = UserErrorTracker(code);
    SemanticAnalyzer analyze_semantics(&user_error_tracker);
    analyze_semantics.analyze_semantics(&ast);
    ASSERT_FALSE(user_error_tracker.has_errors());

    TypeChecker type_checker(&user_error_tracker);
    type_checker.check_types(&ast);
    ASSERT_TRUE(user_error_tracker.has_errors());
}

TEST(FunctionTest, StoreReturnWithIncorrectVarType)
{
    auto code = "fn function() -> int {return 3;} "
                "var result: str = function();";
    auto ast = parse_code(code);

    auto user_error_tracker = UserErrorTracker(code);
    SemanticAnalyzer analyze_semantics(&user_error_tracker);
    analyze_semantics.analyze_semantics(&ast);
    ASSERT_FALSE(user_error_tracker.has_errors());

    TypeChecker type_checker(&user_error_tracker);
    type_checker.check_types(&ast);
    ASSERT_TRUE(user_error_tracker.has_errors());
}

TEST(FunctionTest, ArityFail)
{
    auto code = "fn function(i: int, j: str) {}"
                "function(4, 6, 7);";
    auto ast = parse_code(code);

    auto user_error_tracker = UserErrorTracker(code);
    SemanticAnalyzer analyze_semantics(&user_error_tracker);
    analyze_semantics.analyze_semantics(&ast);
    ASSERT_TRUE(user_error_tracker.has_errors());
}

TEST(FunctionTest, FunctionRedeclaration)
{
    auto code = "fn x() -> int {return 3;}"
                "fn x() -> int {return 1;}";
    auto ast = parse_code(code);

    auto user_error_tracker = UserErrorTracker(code);
    SemanticAnalyzer analyze_semantics(&user_error_tracker);
    analyze_semantics.analyze_semantics(&ast);
    ASSERT_TRUE(user_error_tracker.has_errors());
}