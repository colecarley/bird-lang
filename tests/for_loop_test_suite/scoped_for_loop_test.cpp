#include <gtest/gtest.h>
#include "helpers/compile_helper.hpp"

TEST(ForLoopTest, ScopedForLoopIncrement)
{
    BirdTest::TestOptions options;
    options.code = "var x = 0;"
                   "{"
                   "   for var y = 0; y <= 5; y += 1 do {"
                   "       x = y;"
                   "   }"
                   "}";

    options.compile = false;

    options.after_interpret = [&](Interpreter &interpreter)
    {
        ASSERT_TRUE(interpreter.env.contains("x"));
        ASSERT_TRUE(is_type<int>(interpreter.env.get("x")));
        EXPECT_EQ(as_type<int>(interpreter.env.get("x")), 5);
    };

    ASSERT_TRUE(BirdTest::compile(options));
}
