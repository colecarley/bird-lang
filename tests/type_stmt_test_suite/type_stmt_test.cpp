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

TEST(TypeStmtTest, TypeRedeclaration)
{
    auto code = "type x = int;"
                "type x = bool;";
    auto ast = parse_code(code);

    auto user_error_tracker = UserErrorTracker(code);
    SemanticAnalyzer analyze_semantics(&user_error_tracker);
    analyze_semantics.analyze_semantics(&ast);
    ASSERT_TRUE(user_error_tracker.has_errors());
}

TEST(TypeStmtTest, TypeStmtWithTypeLiteral)
{
    auto code = "type x = int;";
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

    ASSERT_TRUE(interpreter.type_table.contains("x"));
    ASSERT_EQ(interpreter.type_table.get("x").type.lexeme, "int");
}

TEST(TypeStmtTest, TypeStmtWithTypeIdentifier)
{
    auto code = "type x = int;"
                "type y = x;";
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

    ASSERT_TRUE(interpreter.type_table.contains("y"));
    ASSERT_EQ(interpreter.type_table.get("y").type.lexeme, "int");
}

TEST(TypeStmtTest, DeclStmtWithTypeIdentifer)
{
    auto code = "type x = int;"
                "var y: x = 2;";
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

    ASSERT_TRUE(interpreter.env.contains("y"));
    ASSERT_TRUE(is_type<int>(interpreter.env.get("y")));
    ASSERT_EQ(as_type<int>(interpreter.env.get("y")), 2);
}

TEST(TypeStmtTest, ConstStmtWithTypeIdentifer)
{
    auto code = "type x = int;"
                "const y: x = 2;";
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

    ASSERT_TRUE(interpreter.env.contains("y"));
    ASSERT_TRUE(is_type<int>(interpreter.env.get("y")));
    ASSERT_EQ(as_type<int>(interpreter.env.get("y")), 2);
}

TEST(TypeStmtTest, TypeDeclIdentiferRedeclaration)
{
    auto code = "type x = int;"
                "var x = 2;";
    auto ast = parse_code(code);

    auto user_error_tracker = UserErrorTracker(code);
    SemanticAnalyzer analyze_semantics(&user_error_tracker);
    analyze_semantics.analyze_semantics(&ast);
    ASSERT_TRUE(user_error_tracker.has_errors());
}

TEST(TypeStmtTest, FuncTypeIdentiferRedeclaration)
{
    auto code = "type x = int;"
                "fn x() -> int {return 3;};";
    auto ast = parse_code(code);

    auto user_error_tracker = UserErrorTracker(code);
    SemanticAnalyzer analyze_semantics(&user_error_tracker);
    analyze_semantics.analyze_semantics(&ast);
    ASSERT_TRUE(user_error_tracker.has_errors());
}

TEST(TypeStmtTest, DeclTypeMismatch)
{
    auto code = "type x = int;"
                "var y: x = true;";
    auto ast = parse_code(code);

    auto user_error_tracker = UserErrorTracker(code);
    SemanticAnalyzer analyze_semantics(&user_error_tracker);
    analyze_semantics.analyze_semantics(&ast);
    ASSERT_FALSE(user_error_tracker.has_errors());

    TypeChecker type_checker(&user_error_tracker);
    type_checker.check_types(&ast);
    ASSERT_TRUE(user_error_tracker.has_errors());
}