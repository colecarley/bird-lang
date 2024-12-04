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

TEST(SymbolTableTest, ConstScope)
{
    auto code = "const x = 0;"
                "{"
                "x = 1;"
                "}";
    auto ast = parse_code(code);

    auto user_error_tracker = UserErrorTracker(code);
    SemanticAnalyzer analyze_semantics(&user_error_tracker);
    analyze_semantics.analyze_semantics(&ast);
    ASSERT_TRUE(user_error_tracker.has_errors());
}
