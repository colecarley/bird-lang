#include <gtest/gtest.h>
#include <vector>
#include <memory>
#include "../../include/visitors/interpreter.h"
#include "../../src/callable.cpp"
#include "../helpers/parse_test_helper.hpp"
#include "../../include/visitors/semantic_analyzer.h"
#include "../../include/visitors/type_checker.h"

TEST(VarTest, VarRedeclaration)
{
    auto code = "const x = 0;"
                "const x = 1;";
    auto ast = parse_code(code);

    auto user_error_tracker = UserErrorTracker(code);
    SemanticAnalyzer analyze_semantics(&user_error_tracker);
    analyze_semantics.analyze_semantics(&ast);
    ASSERT_FALSE(user_error_tracker.has_errors());

    TypeChecker type_checker(&user_error_tracker);
    type_checker.check_types(&ast);
    ASSERT_TRUE(user_error_tracker.has_errors());
}
