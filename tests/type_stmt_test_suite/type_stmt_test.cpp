#include <gtest/gtest.h>
#include "helpers/compile_helper.hpp"

TEST(TypeStmtTest, TypeRedeclaration)
{
    BirdTest::TestOptions options;
    options.code = "type x = int;"
                   "type x = bool;";

    options.after_semantic_analyze = [&](UserErrorTracker &error_tracker, SemanticAnalyzer &analyzer)
    {
        ASSERT_TRUE(error_tracker.has_errors());
        auto tup = error_tracker.get_errors()[0];

        ASSERT_EQ(std::get<1>(tup).lexeme, "x");
        ASSERT_EQ(std::get<0>(tup), ">>[ERROR] semantic error: Identifier 'x' is already declared. (line 0, character 19)");
    };

    ASSERT_FALSE(BirdTest::compile(options));
}

TEST(TypeStmtTest, TypeStmtWithTypeLiteral)
{
    BirdTest::TestOptions options;
    options.code = "type x = int;";

    options.after_interpret = [&](Interpreter &interpreter)
    {
        ASSERT_TRUE(interpreter.type_table.contains("x"));
        ASSERT_EQ(interpreter.type_table.get("x").type.lexeme, "int");
    };

    options.after_compile = [&](std::string &output, CodeGen &codegen)
    {
        ASSERT_TRUE(codegen.type_table.contains("x"));
        ASSERT_EQ(codegen.type_table.get("x").type.lexeme, "int");
    };

    ASSERT_TRUE(BirdTest::compile(options));
}

TEST(TypeStmtTest, TypeStmtWithTypeIdentifier)
{
    BirdTest::TestOptions options;
    options.code = "type x = int;"
                   "type y = x;";

    options.after_interpret = [&](Interpreter &interpreter)
    {
        ASSERT_TRUE(interpreter.type_table.contains("y"));
        ASSERT_EQ(interpreter.type_table.get("y").type.lexeme, "int");
    };

    options.after_compile = [&](std::string &output, CodeGen &codegen)
    {
        ASSERT_TRUE(codegen.type_table.contains("x"));
        ASSERT_EQ(codegen.type_table.get("y").type.lexeme, "int");
    };

    ASSERT_TRUE(BirdTest::compile(options));
}

TEST(TypeStmtTest, DeclStmtWithTypeIdentifer)
{
    BirdTest::TestOptions options;
    options.code = "type x = int;"
                   "var y: x = 2;"
                   "print y;";

    options.after_interpret = [&](Interpreter &interpreter)
    {
        ASSERT_TRUE(interpreter.env.contains("y"));
        ASSERT_TRUE(is_type<int>(interpreter.env.get("y")));
        ASSERT_EQ(as_type<int>(interpreter.env.get("y")), 2);
    };

    options.after_compile = [&](std::string &output, CodeGen &codegen)
    {
        ASSERT_TRUE(codegen.environment.contains("y"));
        ASSERT_EQ(codegen.environment.get("y").type, CodeGenInt);
        ASSERT_EQ(codegen.environment.get("y").value, 0);
        ASSERT_EQ(output, "2\n\n");
    };

    ASSERT_TRUE(BirdTest::compile(options));
}

TEST(TypeStmtTest, ConstStmtWithTypeIdentifer)
{
    BirdTest::TestOptions options;
    options.code = "type x = int;"
                   "const y: x = 2;"
                   "print y;";

    options.after_interpret = [&](Interpreter &interpreter)
    {
        ASSERT_TRUE(interpreter.env.contains("y"));
        ASSERT_TRUE(is_type<int>(interpreter.env.get("y")));
        ASSERT_EQ(as_type<int>(interpreter.env.get("y")), 2);
    };

    options.after_compile = [&](std::string &output, CodeGen &codegen)
    {
        ASSERT_TRUE(codegen.environment.contains("y"));
        ASSERT_EQ(codegen.environment.get("y").type, CodeGenInt);
        ASSERT_EQ(codegen.environment.get("y").value, 0);
        ASSERT_EQ(output, "2\n\n");
    };

    ASSERT_TRUE(BirdTest::compile(options));
}

// TEST(TypeStmtTest, TypeDeclIdentiferRedeclaration)
// {
//     BirdTest::TestOptions options;
//     options.code = "type x = int;"
//                 "var x = 2;";

//     auto user_error_tracker = UserErrorTracker(code);
//     SemanticAnalyzer analyze_semantics(&user_error_tracker);
//     analyze_semantics.analyze_semantics(&ast);
//     ASSERT_TRUE(user_error_tracker.has_errors());
// }

// TEST(TypeStmtTest, FuncTypeIdentiferRedeclaration)
// {
//     BirdTest::TestOptions options;
//     options.code = "type x = int;"
//                 "fn x() -> int {return 3;};";

//     auto user_error_tracker = UserErrorTracker(code);
//     SemanticAnalyzer analyze_semantics(&user_error_tracker);
//     analyze_semantics.analyze_semantics(&ast);
//     ASSERT_TRUE(user_error_tracker.has_errors());
// }

// TEST(TypeStmtTest, DeclTypeMismatch)
// {
//     BirdTest::TestOptions options;
//     options.code = "type x = int;"
//                 "var y: x = true;";

//     auto user_error_tracker = UserErrorTracker(code);
//     SemanticAnalyzer analyze_semantics(&user_error_tracker);
//     analyze_semantics.analyze_semantics(&ast);
//     ASSERT_FALSE(user_error_tracker.has_errors());

//     TypeChecker type_checker(&user_error_tracker);
//     type_checker.check_types(&ast);
//     ASSERT_TRUE(user_error_tracker.has_errors());
// }