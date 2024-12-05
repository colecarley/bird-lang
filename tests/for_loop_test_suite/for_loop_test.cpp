#include <gtest/gtest.h>
#include "helpers/compile_helper.hpp"

TEST(ForLoopTest, ForLoopIncrement)
{
    BirdTest::TestOptions options;
    options.code = "var z = 0;"
                   "for var x: int = 0; x <= 5; x += 1 do {"
                   "z = x;"
                   "}";

    options.compile = false;

    options.after_interpret = [&](Interpreter &interpreter)
    {
        ASSERT_TRUE(interpreter.env.contains("z"));
        ASSERT_TRUE(is_type<int>(interpreter.env.get("z")));
        EXPECT_EQ(as_type<int>(interpreter.env.get("z")), 5);
    };

    ASSERT_TRUE(BirdTest::compile(options));
}

TEST(ForLoopTest, BreakOutsideLoop)
{
    BirdTest::TestOptions options;
    options.code = "break;";

    options.compile = false;
    options.interpret = false;

    options.after_semantic_analyze = [&](UserErrorTracker &error_tracker, SemanticAnalyzer &analyzer)
    {
        ASSERT_TRUE(error_tracker.has_errors());
        auto tup = error_tracker.get_errors()[0];

        ASSERT_EQ(std::get<1>(tup).lexeme, "break");
        ASSERT_EQ(std::get<0>(tup), ">>[ERROR] semantic error: Break statement is declared outside of a loop. (line 0, character 5)");
    };

    ASSERT_FALSE(BirdTest::compile(options));
}

TEST(ForLoopTest, ContinueOutsideLoop)
{
    BirdTest::TestOptions options;
    options.code = "continue;";

    options.compile = false;
    options.interpret = false;

    options.after_semantic_analyze = [&](UserErrorTracker &error_tracker, SemanticAnalyzer &analyzer)
    {
        ASSERT_TRUE(error_tracker.has_errors());
        auto tup = error_tracker.get_errors()[0];

        ASSERT_EQ(std::get<1>(tup).lexeme, "continue");
        ASSERT_EQ(std::get<0>(tup), ">>[ERROR] semantic error: Continue statement is declared outside of a loop. (line 0, character 8)");
    };

    ASSERT_FALSE(BirdTest::compile(options));
}

TEST(ForLoopTest, ForLoopBreak)
{
    BirdTest::TestOptions options;
    options.code = "var z = 0;"
                   "for var x: int = 0; x <= 5; x += 1 do "
                   "{"
                   "z = x;"
                   "if z == 2 "
                   "{"
                   "break;"
                   "}"
                   "}";

    options.compile = false;

    options.after_interpret = [&](Interpreter &interpreter)
    {
        ASSERT_TRUE(interpreter.env.contains("z"));
        ASSERT_TRUE(is_type<int>(interpreter.env.get("z")));
        EXPECT_EQ(as_type<int>(interpreter.env.get("z")), 2);
    };

    ASSERT_TRUE(BirdTest::compile(options));
}

TEST(ForLoopTest, ForLoopContinue)
{
    BirdTest::TestOptions options;
    options.code = "var z = 0;"
                   "for var x: int = 0; x < 5; x += 1 do {"
                   "if z == 3 {continue;}"
                   "z += 1;"
                   "continue;"
                   "}";

    options.compile = false;

    options.after_interpret = [&](Interpreter &interpreter)
    {
        ASSERT_TRUE(interpreter.env.contains("z"));
        ASSERT_TRUE(is_type<int>(interpreter.env.get("z")));
        EXPECT_EQ(as_type<int>(interpreter.env.get("z")), 3);
    };

    ASSERT_TRUE(BirdTest::compile(options));
}
