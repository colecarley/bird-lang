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

TEST(ForLoopTest, ForLoopIncrement)
{
    auto code = "var z = 0;"
                "for var x: int = 0; x <= 5; x += 1 do {"
                "z = x;"
                "}";

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

    ASSERT_TRUE(interpreter.environment->contains("z"));
    ASSERT_TRUE(is_type<int>(interpreter.environment->get("z")));
    EXPECT_EQ(as_type<int>(interpreter.environment->get("z").data), 5);
}

TEST(ForLoopTest, BreakOutsideLoop)
{
    auto code = "break;";

    auto ast = parse_code(code);

    auto user_error_tracker = UserErrorTracker(code);
    SemanticAnalyzer analyze_semantics(&user_error_tracker);
    analyze_semantics.analyze_semantics(&ast);
    ASSERT_FALSE(user_error_tracker.has_errors());

    TypeChecker type_checker(&user_error_tracker);
    type_checker.check_types(&ast);
    ASSERT_FALSE(user_error_tracker.has_errors());
}

TEST(ForLoopTest, ContinueOutsideLoop)
{
    auto code = "continue;";

    auto ast = parse_code(code);

    auto user_error_tracker = UserErrorTracker(code);
    SemanticAnalyzer analyze_semantics(&user_error_tracker);
    analyze_semantics.analyze_semantics(&ast);
    ASSERT_FALSE(user_error_tracker.has_errors());

    TypeChecker type_checker(&user_error_tracker);
    type_checker.check_types(&ast);
    ASSERT_FALSE(user_error_tracker.has_errors());
}

TEST(ForLoopTest, ForLoopBreak)
{
    auto code = "var z = 0;"
                "for var x: int = 0; x <= 5; x += 1 do "
                "{"
                "z = x;"
                "if z == 2 "
                "{"
                "break;"
                "}"
                "}";

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

    ASSERT_TRUE(interpreter.environment->contains("z"));
    ASSERT_TRUE(is_type<int>(interpreter.environment->get("z")));
    EXPECT_EQ(as_type<int>(interpreter.environment->get("z")), 2);
}

TEST(ForLoopTest, ForLoopContinue)
{
    auto code = "var z = 0;for var x: int = 0; x < 5; x += 1 do {if z == 3 {continue;}z += 1;continue;}";

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

    ASSERT_TRUE(interpreter.environment->contains("z"));
    ASSERT_TRUE(is_type<int>(interpreter.environment->get("z")));
    EXPECT_EQ(as_type<int>(interpreter.environment->get("z").data), 3);
}
