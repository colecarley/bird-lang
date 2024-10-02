#include <gtest/gtest.h>
#include "../src/lexer.cpp"
#include "../include/exceptions/user_error_tracker.h"

// helper function prototypes
std::vector<Token> lex_code(const std::string &code);

TEST(LexerTest, TokenizeBlockStmt)
{
    std::string code = "{ var x: int = 10; var y: int = 20}";

    auto tokens = lex_code(code);

    EXPECT_EQ(tokens.size(), 16);
    EXPECT_EQ(tokens[0].token_type, Token::Type::LBRACE);
    EXPECT_EQ(tokens[1].token_type, Token::Type::VAR);
    EXPECT_EQ(tokens[2].token_type, Token::Type::IDENTIFIER);
    EXPECT_EQ(tokens[2].lexeme, "x");
    EXPECT_EQ(tokens[3].token_type, Token::Type::COLON);
    EXPECT_EQ(tokens[4].token_type, Token::Type::TYPE_IDENTIFIER);
    EXPECT_EQ(tokens[4].lexeme, "int");
    EXPECT_EQ(tokens[5].token_type, Token::Type::EQUAL);
    EXPECT_EQ(tokens[6].token_type, Token::Type::INT_LITERAL);
    EXPECT_EQ(tokens[6].lexeme, "10");
    EXPECT_EQ(tokens[7].token_type, Token::Type::SEMICOLON);
    EXPECT_EQ(tokens[8].token_type, Token::Type::VAR);
    EXPECT_EQ(tokens[9].token_type, Token::Type::IDENTIFIER);
    EXPECT_EQ(tokens[9].lexeme, "y");
    EXPECT_EQ(tokens[10].token_type, Token::Type::COLON);
    EXPECT_EQ(tokens[11].token_type, Token::Type::TYPE_IDENTIFIER);
    EXPECT_EQ(tokens[11].lexeme, "int");
    EXPECT_EQ(tokens[12].token_type, Token::Type::EQUAL);
    EXPECT_EQ(tokens[13].token_type, Token::Type::INT_LITERAL);
    EXPECT_EQ(tokens[13].lexeme, "20");
    EXPECT_EQ(tokens[14].token_type, Token::Type::RBRACE);
    EXPECT_EQ(tokens[15].token_type, Token::Type::END);
}

TEST(LexerTest, TokenizeConstStmt)
{
    std::string code = "const z: str = \"foobar\";";

    auto tokens = lex_code(code);

    EXPECT_EQ(tokens.size(), 8);
    EXPECT_EQ(tokens[0].token_type, Token::Type::CONST);
    EXPECT_EQ(tokens[1].token_type, Token::Type::IDENTIFIER);
    EXPECT_EQ(tokens[1].lexeme, "z");
    EXPECT_EQ(tokens[2].token_type, Token::Type::COLON);
    EXPECT_EQ(tokens[3].token_type, Token::Type::TYPE_IDENTIFIER);
    EXPECT_EQ(tokens[3].lexeme, "str");
    EXPECT_EQ(tokens[4].token_type, Token::Type::EQUAL);
    EXPECT_EQ(tokens[5].token_type, Token::Type::STR_LITERAL);
    EXPECT_EQ(tokens[5].lexeme, "foobar");
    EXPECT_EQ(tokens[6].token_type, Token::Type::SEMICOLON);
    EXPECT_EQ(tokens[7].token_type, Token::Type::END);
}

TEST(LexerTest, TokenizeDeclStmt)
{
    std::string code = "var x: int = 3;";

    auto tokens = lex_code(code);

    EXPECT_EQ(tokens.size(), 8);
    EXPECT_EQ(tokens[0].token_type, Token::Type::VAR);
    EXPECT_EQ(tokens[1].token_type, Token::Type::IDENTIFIER);
    EXPECT_EQ(tokens[1].lexeme, "x");
    EXPECT_EQ(tokens[2].token_type, Token::Type::COLON);
    EXPECT_EQ(tokens[3].token_type, Token::Type::TYPE_IDENTIFIER);
    EXPECT_EQ(tokens[3].lexeme, "int");
    EXPECT_EQ(tokens[4].token_type, Token::Type::EQUAL);
    EXPECT_EQ(tokens[5].token_type, Token::Type::INT_LITERAL);
    EXPECT_EQ(tokens[5].lexeme, "3");
    EXPECT_EQ(tokens[6].token_type, Token::Type::SEMICOLON);
    EXPECT_EQ(tokens[7].token_type, Token::Type::END);
}

TEST(LexerTest, TokenizeExprStmt)
{
    std::string code = "1 + 2 - 3 / 6 * 4 <= 4";

    auto tokens = lex_code(code);

    EXPECT_EQ(tokens.size(), 12);
    EXPECT_EQ(tokens[0].token_type, Token::Type::INT_LITERAL);
    EXPECT_EQ(tokens[0].lexeme, "1");
    EXPECT_EQ(tokens[1].token_type, Token::Type::PLUS);
    EXPECT_EQ(tokens[2].token_type, Token::Type::INT_LITERAL);
    EXPECT_EQ(tokens[2].lexeme, "2");
    EXPECT_EQ(tokens[3].token_type, Token::Type::MINUS);
    EXPECT_EQ(tokens[4].token_type, Token::Type::INT_LITERAL);
    EXPECT_EQ(tokens[4].lexeme, "3");
    EXPECT_EQ(tokens[5].token_type, Token::Type::SLASH);
    EXPECT_EQ(tokens[6].token_type, Token::Type::INT_LITERAL);
    EXPECT_EQ(tokens[6].lexeme, "6");
    EXPECT_EQ(tokens[7].token_type, Token::Type::STAR);
    EXPECT_EQ(tokens[8].token_type, Token::Type::INT_LITERAL);
    EXPECT_EQ(tokens[8].lexeme, "4");
    EXPECT_EQ(tokens[9].token_type, Token::Type::LESS_EQUAL);
    EXPECT_EQ(tokens[10].token_type, Token::Type::INT_LITERAL);
    EXPECT_EQ(tokens[10].lexeme, "4");
    EXPECT_EQ(tokens[11].token_type, Token::Type::END);
}

TEST(LexerTest, TokenizeFuncStmt)
{
    std::string code = "fn add(first: int, second: int) -> int { return first + second; }";

    auto tokens = lex_code(code);

    EXPECT_EQ(tokens.size(), 21);
    EXPECT_EQ(tokens[0].token_type, Token::Type::FN);
    EXPECT_EQ(tokens[1].token_type, Token::Type::IDENTIFIER);
    EXPECT_EQ(tokens[1].lexeme, "add");
    EXPECT_EQ(tokens[2].token_type, Token::Type::LPAREN);
    EXPECT_EQ(tokens[3].token_type, Token::Type::IDENTIFIER);
    EXPECT_EQ(tokens[3].lexeme, "first");
    EXPECT_EQ(tokens[4].token_type, Token::Type::COLON);
    EXPECT_EQ(tokens[5].token_type, Token::Type::TYPE_IDENTIFIER);
    EXPECT_EQ(tokens[5].lexeme, "int");
    EXPECT_EQ(tokens[6].token_type, Token::Type::COMMA);
    EXPECT_EQ(tokens[7].token_type, Token::Type::IDENTIFIER);
    EXPECT_EQ(tokens[7].lexeme, "second");
    EXPECT_EQ(tokens[8].token_type, Token::Type::COLON);
    EXPECT_EQ(tokens[9].token_type, Token::Type::TYPE_IDENTIFIER);
    EXPECT_EQ(tokens[9].lexeme, "int");
    EXPECT_EQ(tokens[10].token_type, Token::Type::RPAREN);
    EXPECT_EQ(tokens[11].token_type, Token::Type::ARROW);
    EXPECT_EQ(tokens[12].token_type, Token::Type::TYPE_IDENTIFIER);
    EXPECT_EQ(tokens[12].lexeme, "int");
    EXPECT_EQ(tokens[13].token_type, Token::Type::LBRACE);
    EXPECT_EQ(tokens[14].token_type, Token::Type::RETURN);
    EXPECT_EQ(tokens[15].token_type, Token::Type::IDENTIFIER);
    EXPECT_EQ(tokens[15].lexeme, "first");
    EXPECT_EQ(tokens[16].token_type, Token::Type::PLUS);
    EXPECT_EQ(tokens[17].token_type, Token::Type::IDENTIFIER);
    EXPECT_EQ(tokens[17].lexeme, "second");
    EXPECT_EQ(tokens[18].token_type, Token::Type::SEMICOLON);
    EXPECT_EQ(tokens[19].token_type, Token::Type::RBRACE);
    EXPECT_EQ(tokens[20].token_type, Token::Type::END);
}

TEST(LexerTest, TokenizeIfStmt)
{
    std::string code = "if 1 > 2 print 1; else if 1 == 2 print 2; else print 3;";

    auto tokens = lex_code(code);

    EXPECT_EQ(tokens.size(), 20);
    EXPECT_EQ(tokens[0].token_type, Token::Type::IF);
    EXPECT_EQ(tokens[1].token_type, Token::Type::INT_LITERAL);
    EXPECT_EQ(tokens[1].lexeme, "1");
    EXPECT_EQ(tokens[2].token_type, Token::Type::GREATER);
    EXPECT_EQ(tokens[3].token_type, Token::Type::INT_LITERAL);
    EXPECT_EQ(tokens[3].lexeme, "2");
    EXPECT_EQ(tokens[4].token_type, Token::Type::PRINT);
    EXPECT_EQ(tokens[5].token_type, Token::Type::INT_LITERAL);
    EXPECT_EQ(tokens[5].lexeme, "1");
    EXPECT_EQ(tokens[6].token_type, Token::Type::SEMICOLON);
    EXPECT_EQ(tokens[7].token_type, Token::Type::ELSE);
    EXPECT_EQ(tokens[8].token_type, Token::Type::IF);
    EXPECT_EQ(tokens[9].token_type, Token::Type::INT_LITERAL);
    EXPECT_EQ(tokens[9].lexeme, "1");
    EXPECT_EQ(tokens[10].token_type, Token::Type::EQUAL_EQUAL);
    EXPECT_EQ(tokens[11].token_type, Token::Type::INT_LITERAL);
    EXPECT_EQ(tokens[11].lexeme, "2");
    EXPECT_EQ(tokens[12].token_type, Token::Type::PRINT);
    EXPECT_EQ(tokens[13].token_type, Token::Type::INT_LITERAL);
    EXPECT_EQ(tokens[13].lexeme, "2");
    EXPECT_EQ(tokens[14].token_type, Token::Type::SEMICOLON);
    EXPECT_EQ(tokens[15].token_type, Token::Type::ELSE);
    EXPECT_EQ(tokens[16].token_type, Token::Type::PRINT);
    EXPECT_EQ(tokens[17].token_type, Token::Type::INT_LITERAL);
    EXPECT_EQ(tokens[17].lexeme, "3");
    EXPECT_EQ(tokens[18].token_type, Token::Type::SEMICOLON);
    EXPECT_EQ(tokens[19].token_type, Token::Type::END);
}

TEST(LexerTest, TokenizePrintStmt)
{
    std::string code = "print 28;";

    auto tokens = lex_code(code);

    EXPECT_EQ(tokens.size(), 4);
    EXPECT_EQ(tokens[0].token_type, Token::Type::PRINT);
    EXPECT_EQ(tokens[1].token_type, Token::Type::INT_LITERAL);
    EXPECT_EQ(tokens[1].lexeme, "28");
    EXPECT_EQ(tokens[2].token_type, Token::Type::SEMICOLON);
    EXPECT_EQ(tokens[3].token_type, Token::Type::END);
}

TEST(LexerTest, TokenizeWhileStmt)
{
    std::string code = "while (true) { print 1; }";

    auto tokens = lex_code(code);

    EXPECT_EQ(tokens.size(), 10);
    EXPECT_EQ(tokens[0].token_type, Token::Type::WHILE);
    EXPECT_EQ(tokens[1].token_type, Token::Type::LPAREN);
    EXPECT_EQ(tokens[2].token_type, Token::Type::BOOL_LITERAL);
    EXPECT_EQ(tokens[2].lexeme, "true");
    EXPECT_EQ(tokens[3].token_type, Token::Type::RPAREN);
    EXPECT_EQ(tokens[4].token_type, Token::Type::LBRACE);
    EXPECT_EQ(tokens[5].token_type, Token::Type::PRINT);
    EXPECT_EQ(tokens[6].token_type, Token::Type::INT_LITERAL);
    EXPECT_EQ(tokens[6].lexeme, "1");
    EXPECT_EQ(tokens[7].token_type, Token::Type::SEMICOLON);
    EXPECT_EQ(tokens[8].token_type, Token::Type::RBRACE);
    EXPECT_EQ(tokens[9].token_type, Token::Type::END);
}

// helper function which returns a vector of tokens
std::vector<Token> lex_code(const std::string &code)
{
    UserErrorTracker error_tracker(code);
    Lexer lexer(code, &error_tracker);
    return lexer.lex();
}