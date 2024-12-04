#include <gtest/gtest.h>
#include "helpers/compile_helper.hpp"

TEST(ConstTest, ConstRedeclaration)
{
    BirdTest::TestOptions options;
    options.code = "var x = 0;"
                   "var x = 1;";
    options.type_check = false;
    options.interpret = false;
    options.compile = false;

    options.after_semantic_analyze = [&](UserErrorTracker &error_tracker, SemanticAnalyzer &analyzer)
    {
        ASSERT_TRUE(error_tracker.has_errors());
        auto tup = error_tracker.get_errors()[0];

        ASSERT_EQ(std::get<1>(tup).lexeme, "x");
        ASSERT_EQ(std::get<0>(tup), ">>[ERROR] semantic error: Identifier 'x' is already declared. (line 0, character 19)");
    };

    ASSERT_FALSE(BirdTest::compile(options));

    options.after_semantic_analyze = [&](UserErrorTracker &error_tracker, SemanticAnalyzer &analyze_semantics)
    {
        ASSERT_TRUE(error_tracker.has_errors());
    };

    BirdTest::compile(options);
}

TEST(ConstTest, Foobar)
{
    BirdTest::TestOptions options;
    options.code = "var x = 0; print x; print x + 1;";

    options.after_interpret = [&](Interpreter &interpreter)
    {
        ASSERT_TRUE(interpreter.env.contains("x"));
        ASSERT_TRUE(is_type<int>(interpreter.env.get("x")));
        ASSERT_EQ(as_type<int>(interpreter.env.get("x")), 0);
    };

    options.after_compile = [&](std::string &output)
    {
        ASSERT_EQ(output, "0\n1\n\n");
    };

    BirdTest::compile(options);
}