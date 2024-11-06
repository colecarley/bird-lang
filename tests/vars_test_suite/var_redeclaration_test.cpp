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

TEST(VarTest, VarRedeclaration)
{
    auto code = "var x = 0;"
                "var x = 1;";
    auto ast = parse_code(code);

    auto user_error_tracker = UserErrorTracker(code);
    SemanticAnalyzer analyze_semantics(&user_error_tracker);
    analyze_semantics.analyze_semantics(&ast);
    ASSERT_TRUE(user_error_tracker.has_errors());
}
