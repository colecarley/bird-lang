#include <gtest/gtest.h>
#include "helpers/compile_helper.hpp"

TEST(WhileTest, While)
{
    BirdTest::TestOptions options;
    options.code = "var x = 0; while x < 10 { x += 1; }";

    options.after_interpret = [&](Interpreter &interpreter)
    {
        ASSERT_TRUE(interpreter.env.contains("x"));
        ASSERT_TRUE(is_type<int>(interpreter.env.get("x")));
        ASSERT_EQ(as_type<int>(interpreter.env.get("x")), 10);
    };

    ASSERT_TRUE(BirdTest::compile(options));
}

TEST(WhileTest, WhileFalse)
{
    BirdTest::TestOptions options;
    options.code = "var x = 0; while false { x = 1; }";

    options.after_interpret = [&](Interpreter &interpreter)
    {
        ASSERT_TRUE(interpreter.env.contains("x"));
        ASSERT_TRUE(is_type<int>(interpreter.env.get("x")));
        ASSERT_EQ(as_type<int>(interpreter.env.get("x")), 0);
    };

    ASSERT_TRUE(BirdTest::compile(options));
}

TEST(WhileTest, WhileBreak)
{
    BirdTest::TestOptions options;
    options.code = "var x = 1; while true { x = 2; break; }";

    options.after_interpret = [&](Interpreter &interpreter)
    {
        ASSERT_TRUE(interpreter.env.contains("x"));
        ASSERT_TRUE(is_type<int>(interpreter.env.get("x")));
        ASSERT_EQ(as_type<int>(interpreter.env.get("x")), 2);
    };

    ASSERT_TRUE(BirdTest::compile(options));
}

TEST(WhileTest, WhileContinue)
{
    BirdTest::TestOptions options;
    options.code = "var x = 0; while true { x += 1; if (x <= 2) { continue; } break; }";

    options.after_interpret = [&](Interpreter &interpreter)
    {
        ASSERT_TRUE(interpreter.env.contains("x"));
        ASSERT_TRUE(is_type<int>(interpreter.env.get("x")));
        ASSERT_EQ(as_type<int>(interpreter.env.get("x")), 3);
    };

    ASSERT_TRUE(BirdTest::compile(options));
}

TEST(WhileTest, WhileConstInc)
{
    BirdTest::TestOptions options;
    options.code = "const inc = 1; var x = 0; while x < 10 { x += inc; }";

    options.after_interpret = [&](Interpreter &interpreter)
    {
        ASSERT_TRUE(interpreter.env.contains("x"));
        ASSERT_TRUE(is_type<int>(interpreter.env.get("x")));
        ASSERT_EQ(as_type<int>(interpreter.env.get("x")), 10);

        ASSERT_TRUE(interpreter.env.contains("inc"));
        ASSERT_TRUE(is_type<int>(interpreter.env.get("inc")));
        ASSERT_EQ(as_type<int>(interpreter.env.get("inc")), 1);
    };

    ASSERT_TRUE(BirdTest::compile(options));
}
