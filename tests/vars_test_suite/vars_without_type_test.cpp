#include <gtest/gtest.h>
#include "helpers/compile_helper.hpp"

// FLOATS
TEST(VarTest, VarWithoutTypeFloat)
{
    BirdTest::TestOptions options;
    options.code = "var x = 4.0;";

    options.after_interpret = [&](Interpreter &interpreter)
    {
        ASSERT_TRUE(interpreter.env.contains("x"));
        ASSERT_TRUE(is_type<double>(interpreter.env.get("x")));
        ASSERT_EQ(as_type<double>(interpreter.env.get("x")), 4.0);
    };

    ASSERT_TRUE(BirdTest::compile(options));
}

// INTS
TEST(VarTest, VarWithoutTypeInt)
{
    BirdTest::TestOptions options;
    options.code = "var x = 4;";

    options.after_interpret = [&](Interpreter &interpreter)
    {
        ASSERT_TRUE(interpreter.env.contains("x"));
        ASSERT_TRUE(is_type<int>(interpreter.env.get("x")));
        ASSERT_EQ(as_type<int>(interpreter.env.get("x")), 4);
    };

    ASSERT_TRUE(BirdTest::compile(options));
}

// STRINGS
TEST(VarTest, VarWithoutTypeString)
{
    BirdTest::TestOptions options;
    options.code = "var x = \"hello\";";

    options.after_interpret = [&](Interpreter &interpreter)
    {
        ASSERT_TRUE(interpreter.env.contains("x"));
        ASSERT_TRUE(is_type<std::string>(interpreter.env.get("x")));
        ASSERT_EQ(as_type<std::string>(interpreter.env.get("x")), "hello");
    };

    ASSERT_TRUE(BirdTest::compile(options));
}

// BOOLS
TEST(VarTest, VarWithoutTypeBoolTrue)
{
    BirdTest::TestOptions options;
    options.code = "var x = true;";

    options.after_interpret = [&](Interpreter &interpreter)
    {
        ASSERT_TRUE(interpreter.env.contains("x"));
        ASSERT_TRUE(is_type<bool>(interpreter.env.get("x")));
        ASSERT_EQ(as_type<bool>(interpreter.env.get("x")), true);
    };

    ASSERT_TRUE(BirdTest::compile(options));
}

TEST(VarTest, VarWithoutTypeBoolFalse)
{
    BirdTest::TestOptions options;
    options.code = "var y = false;";

    options.after_interpret = [&](Interpreter &interpreter)
    {
        ASSERT_TRUE(interpreter.env.contains("y"));
        ASSERT_TRUE(is_type<bool>(interpreter.env.get("y")));
        ASSERT_EQ(as_type<bool>(interpreter.env.get("y")), false);
    };

    ASSERT_TRUE(BirdTest::compile(options));
}
