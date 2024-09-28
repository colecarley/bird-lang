#include <gtest/gtest.h>
#include "src/lexer.cpp"
#include "src/parser.cpp"
#include "include/exceptions/user_error_tracker.h"
// Demonstrate some basic assertions.
// TEST(HelloTest, BasicAssertions)
// {
//   // Expect two strings not to be equal.
//   EXPECT_STRNE("hello", "world");
//   // Expect equality.
//   EXPECT_EQ(7 * 6, 42);
// }

// helper function prototypes
std::vector<Token> lex_code(const std::string &code);
std::vector<std::unique_ptr<Stmt>> parse_code(const std::string &code);
void print_expr(Expr *expr);

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

TEST(LexerTest, TokenizeConstStmt)
{
  std::string code = "const z: str = \"foobar\";";

  auto tokens = lex_code(code);
  ;

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

TEST(LexerTest, TokenizeExprStmt)
{
  std::string code = "1 + 2 - 3 / 2 * 4 <= 4";

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
  EXPECT_EQ(tokens[6].lexeme, "2");
  EXPECT_EQ(tokens[7].token_type, Token::Type::STAR);
  EXPECT_EQ(tokens[8].token_type, Token::Type::INT_LITERAL);
  EXPECT_EQ(tokens[8].lexeme, "4");
  EXPECT_EQ(tokens[9].token_type, Token::Type::LESS_EQUAL);
  EXPECT_EQ(tokens[10].token_type, Token::Type::INT_LITERAL);
  EXPECT_EQ(tokens[10].lexeme, "4");
  EXPECT_EQ(tokens[11].token_type, Token::Type::END);
}

TEST(LexerTest, TokenizeFunc)
{
  std::string code = "fn add(first: int, second: int): int { return first + second; }";

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
  EXPECT_EQ(tokens[11].token_type, Token::Type::COLON);
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

TEST(LexerTest, TokenizePrintStmt)
{
  /*
   * TODO: PRINT STR_LITERAL
   */

  std::string code = "print 28;";

  auto tokens = lex_code(code);

  EXPECT_EQ(tokens.size(), 4);
  EXPECT_EQ(tokens[0].token_type, Token::Type::PRINT);
  EXPECT_EQ(tokens[1].token_type, Token::Type::INT_LITERAL);
  EXPECT_EQ(tokens[1].lexeme, "28");
  EXPECT_EQ(tokens[2].token_type, Token::Type::SEMICOLON);
  EXPECT_EQ(tokens[3].token_type, Token::Type::END);
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

TEST(ParserTest, ParseBlockStmt)
{
  std::string code = "{ var x: int = 10; var y: int = 20; }";

  auto stmts = parse_code(code);

  ASSERT_EQ(stmts.size(), 1); // should parse to 1 block statement

  Block *block = dynamic_cast<Block *>(stmts[0].get()); // verify statement is of type Block
  ASSERT_NE(block, nullptr);                            // if NE will fail since dynamic_cast returned nullptr

  // make sure the block contains 2 statements
  ASSERT_EQ(block->stmts.size(), 2);

  // verify first statement is of type DeclStmt
  DeclStmt *first_decl_stmt = dynamic_cast<DeclStmt *>(block->stmts[0].get());
  ASSERT_NE(first_decl_stmt, nullptr);
  EXPECT_EQ(first_decl_stmt->identifier.lexeme, "x");        // ensure lexeme is correct for identifier
  EXPECT_EQ(first_decl_stmt->type_identifier.lexeme, "int"); // ensure lexeme is correct for type ID

  // ensure primary expression is set
  Primary *first_decl_value = dynamic_cast<Primary *>(first_decl_stmt->value.get());
  ASSERT_NE(first_decl_value, nullptr);
  EXPECT_EQ(first_decl_value->value.lexeme, "10");

  // repeat
  DeclStmt *second_decl_stmt = dynamic_cast<DeclStmt *>(block->stmts[1].get());
  ASSERT_NE(second_decl_stmt, nullptr);
  EXPECT_EQ(second_decl_stmt->identifier.lexeme, "y");
  EXPECT_EQ(second_decl_stmt->type_identifier.lexeme, "int");

  Primary *second_decl_value = dynamic_cast<Primary *>(second_decl_stmt->value.get());
  ASSERT_NE(second_decl_value, nullptr);
  EXPECT_EQ(second_decl_value->value.lexeme, "20");
}

TEST(ParserTest, ParseConstStmt)
{
  std::string code = "const z: str = \"foobar\";";

  auto stmts = parse_code(code);

  ASSERT_EQ(stmts.size(), 1);

  ConstStmt *const_stmt = dynamic_cast<ConstStmt *>(stmts[0].get());
  ASSERT_NE(const_stmt, nullptr);
  EXPECT_EQ(const_stmt->identifier.lexeme, "z");
  EXPECT_EQ(const_stmt->type_identifier.lexeme, "str");

  Primary *primary_expr = dynamic_cast<Primary *>(const_stmt->value.get());
  ASSERT_NE(primary_expr, nullptr);
  EXPECT_EQ(primary_expr->value.lexeme, "foobar");
}

TEST(ParserTest, ParseDeclStmt)
{
  std::string code = "var x: int = 3;";

  auto stmts = parse_code(code);

  ASSERT_EQ(stmts.size(), 1);

  DeclStmt *decl_stmt = dynamic_cast<DeclStmt *>(stmts[0].get());
  ASSERT_NE(decl_stmt, nullptr);
  EXPECT_EQ(decl_stmt->identifier.lexeme, "x");
  EXPECT_EQ(decl_stmt->type_identifier.lexeme, "int");

  Primary *primary_expr = dynamic_cast<Primary *>(decl_stmt->value.get());
  ASSERT_NE(primary_expr, nullptr);
  EXPECT_EQ(primary_expr->value.lexeme, "3");
}

TEST(ParserTest, ParseExprStmt)
{
  /*
   *                       (1 + 2 - 3 / 6 * 4 <=  4)
   *                            /              |   \
   *               (1 + 2 - 3 / 6 * 4)        `<=`  `4`
   *              /       |        \
   *         (1 + 2)     `-`    (3 / 6 * 4)
   *        /   |  \            /      |  \
   *      `1`  `+` `2`      (3  / 6)  `*` `4`
   *                         /  |   \
   *                       `3` `/` `6`
   */
  std::string code = "1 + 2 - 3 / 6 * 4 <= 4;";

  auto stmts = parse_code(code);

  ASSERT_EQ(stmts.size(), 1);
  ExprStmt *expr_stmt = dynamic_cast<ExprStmt *>(stmts[0].get());
  ASSERT_NE(expr_stmt, nullptr);

  // outer binary expression `1 + 2 - 3 / 6 * 4 <= 4`
  Binary *outer_binary_expr = dynamic_cast<Binary *>(expr_stmt->expr.get());
  ASSERT_NE(outer_binary_expr, nullptr);
  EXPECT_EQ(outer_binary_expr->op.lexeme, "<=");

  // rhs LESS_EQUAL ... <= `4`
  Primary *rhs_primary = dynamic_cast<Primary *>(outer_binary_expr->right.get());
  ASSERT_NE(rhs_primary, nullptr);
  EXPECT_EQ(rhs_primary->value.lexeme, "4");

  // lhs of LESS_EQUAL `1 + 2 - 3 / 6 * 4` <= ...
  Binary *lhs_binary_expr = dynamic_cast<Binary *>(outer_binary_expr->left.get());
  ASSERT_NE(lhs_binary_expr, nullptr);
  EXPECT_EQ(lhs_binary_expr->op.lexeme, "-");

  // lhs of MINUS `1 + 2` - ...
  Binary *addition_expr = dynamic_cast<Binary *>(lhs_binary_expr->left.get());
  ASSERT_NE(addition_expr, nullptr);
  EXPECT_EQ(addition_expr->op.lexeme, "+");

  // lhs of PLUS `1` + ...
  Primary *lhs_primary_one = dynamic_cast<Primary *>(addition_expr->left.get());
  ASSERT_NE(lhs_primary_one, nullptr);
  EXPECT_EQ(lhs_primary_one->value.lexeme, "1");

  // rhs of PLUS ... + `2`
  Primary *rhs_primary_two = dynamic_cast<Primary *>(addition_expr->right.get());
  ASSERT_NE(rhs_primary_two, nullptr);
  EXPECT_EQ(rhs_primary_two->value.lexeme, "2");

  // rhs of MINUS ... - `3 / 6 * 4`
  Binary *multiplication_expr = dynamic_cast<Binary *>(lhs_binary_expr->right.get());
  ASSERT_NE(multiplication_expr, nullptr);
  EXPECT_EQ(multiplication_expr->op.lexeme, "*");

  // lhs of STAR `3 / 6` * ...
  Binary *division_expr = dynamic_cast<Binary *>(multiplication_expr->left.get());
  ASSERT_NE(division_expr, nullptr);
  EXPECT_EQ(division_expr->op.lexeme, "/");

  // lhs of SLASH `3` / ...
  Primary *lhs_primary_three = dynamic_cast<Primary *>(division_expr->left.get());
  ASSERT_NE(lhs_primary_three, nullptr);
  EXPECT_EQ(lhs_primary_three->value.lexeme, "3");

  // rhs of SLASH ... / `6`
  Primary *rhs_primary_six = dynamic_cast<Primary *>(division_expr->right.get());
  ASSERT_NE(rhs_primary_six, nullptr);
  EXPECT_EQ(rhs_primary_six->value.lexeme, "6");

  // rhs of STAR ... * `4`
  Primary *rhs_primary_four = dynamic_cast<Primary *>(multiplication_expr->right.get());
  ASSERT_NE(rhs_primary_four, nullptr);
  EXPECT_EQ(rhs_primary_four->value.lexeme, "4");

  // std::cout << "expression: " << std::endl;
  // print_expr(expr_stmt->expr.get());
  // std::cout << std::endl;
}

std::vector<Token> lex_code(const std::string &code)
{
  UserErrorTracker error_tracker(code);
  Lexer lexer(code, &error_tracker);
  return lexer.lex();
}

std::vector<std::unique_ptr<Stmt>> parse_code(const std::string &code)
{
  UserErrorTracker error_tracker(code);
  Lexer lexer(code, &error_tracker);
  std::vector<Token> tokens = lexer.lex();

  Parser parser(tokens, &error_tracker);
  return parser.parse();
}

void print_expr(Expr *expr)
{
  if (auto binary = dynamic_cast<Binary *>(expr))
  {
    std::cout << "(";
    print_expr(binary->left.get());
    std::cout << " " << binary->op.lexeme << " ";
    print_expr(binary->right.get());
    std::cout << ")";
  }
  else if (auto primary = dynamic_cast<Primary *>(expr))
  {
    std::cout << primary->value.lexeme;
  }
}