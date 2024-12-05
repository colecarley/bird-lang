#include <gtest/gtest.h>
#include "helpers/compile_helper.hpp"

// INT
TEST(VarTest, VarWithTypeInt)
{
    BirdTest::TestOptions options;
    options.code = "var x: int = 4;";

    options.after_interpret = [&](Interpreter &interpreter)
    {
        ASSERT_TRUE(interpreter.env.contains("x"));
        ASSERT_TRUE(is_type<int>(interpreter.env.get("x")));
        ASSERT_EQ(as_type<int>(interpreter.env.get("x")), 4);
    };

    ASSERT_TRUE(BirdTest::compile(options));
}

TEST(VarTest, VarWithTypeFloatExplicit)
{
    BirdTest::TestOptions options;
    options.code = "var x: float = 4.0;";

    options.after_interpret = [&](Interpreter &interpreter)
    {
        ASSERT_TRUE(interpreter.env.contains("x"));
        ASSERT_TRUE(is_type<double>(interpreter.env.get("x")));
        ASSERT_EQ(as_type<double>(interpreter.env.get("x")), 4.0);
    };

    ASSERT_TRUE(BirdTest::compile(options));
}

TEST(VarTest, VarWithTypeFloatImplicit)
{
    BirdTest::TestOptions options;
    options.code = "var y: float = 4;";

    options.after_interpret = [&](Interpreter &interpreter)
    {
        ASSERT_TRUE(interpreter.env.contains("y"));
        ASSERT_TRUE(is_type<double>(interpreter.env.get("y")));
        ASSERT_EQ(as_type<double>(interpreter.env.get("y")), 4.0);
    };

    ASSERT_TRUE(BirdTest::compile(options));
}

// STRINGS
TEST(VarTest, VarWithTypeString)
{
    BirdTest::TestOptions options;
    options.code = "var x: str = \"hello\";";

    options.after_interpret = [&](Interpreter &interpreter)
    {
        ASSERT_TRUE(interpreter.env.contains("x"));
        ASSERT_TRUE(is_type<std::string>(interpreter.env.get("x")));
        ASSERT_EQ(as_type<std::string>(interpreter.env.get("x")), "hello");
    };

    ASSERT_TRUE(BirdTest::compile(options));
}

// BOOL
TEST(VarTest, VarWithTypeBoolTrue)
{
    BirdTest::TestOptions options;
    options.code = "var x: bool = true;";

    options.after_interpret = [&](Interpreter &interpreter)
    {
        ASSERT_TRUE(interpreter.env.contains("x"));
        ASSERT_TRUE(is_type<bool>(interpreter.env.get("x")));
        ASSERT_EQ(as_type<bool>(interpreter.env.get("x")), true);
    };

    ASSERT_TRUE(BirdTest::compile(options));
}

TEST(VarTest, VarWithTypeBoolFalse)
{
    BirdTest::TestOptions options;
    options.code = "var y: bool = false;";

    options.after_interpret = [&](Interpreter &interpreter)
    {
        ASSERT_TRUE(interpreter.env.contains("y"));
        ASSERT_TRUE(is_type<bool>(interpreter.env.get("y")));
        ASSERT_EQ(as_type<bool>(interpreter.env.get("y")), false);
    };

    ASSERT_TRUE(BirdTest::compile(options));
}
