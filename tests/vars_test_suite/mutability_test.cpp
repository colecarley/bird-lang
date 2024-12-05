#include <gtest/gtest.h>
#include "helpers/compile_helper.hpp"

TEST(VarTest, VarMutability)
{
    BirdTest::TestOptions options;
    options.code = "var x: int = 4; x = 5;";

    options.after_interpret = [&](Interpreter &interpreter)
    {
        ASSERT_TRUE(interpreter.env.contains("x"));
        ASSERT_TRUE(is_type<int>(interpreter.env.get("x")));
        ASSERT_EQ(as_type<int>(interpreter.env.get("x")), 5);
    };

    ASSERT_TRUE(BirdTest::compile(options));
}
