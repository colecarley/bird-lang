#pragma once

#include <string>
#include <vector>
#include <iostream>
#include <map>

enum TokenType
{
    // keywords
    VAR,
    PRINT,
    IF,
    ELSE,
    WHILE,
    // types
    INT,
    FLOAT,
    STR,
    BOOL,

    // single character
    STAR,
    SLASH,
    PLUS,
    MINUS,
    SEMICOLON,
    EQUAL,
    COMMA,
    RBRACE,
    LBRACE,
    RPAREN,
    LPAREN,
    COLON,
    GREATER,
    LESS,
    BANG,

    // two character
    GREATER_EQUAL,
    LESS_EQUAL,
    EQUAL_EQUAL,
    BANG_EQUAL,

    // values
    IDENTIFIER,
    NUMBER,
    INT_LITERAL,
    FLOAT_LITERAL,
    BOOL_LITERAL,
    STR_LITERAL,

    // end
    END
};

class Token
{
public:
    TokenType token_type;
    std::string lexeme;

    void print_token();

    Token();

    Token(TokenType token_type, std::string lexeme);
};

/*
 * The lexer takes the source code as input,
 * look at each character and group them into tokens
 */
class Lexer
{
public:
    std::string input;
    unsigned int position;
    std::vector<Token> tokens;
    static const std::map<std::string, TokenType> keywords;

    Lexer(std::string input);

    std::vector<Token> lex();

    void handle_number();

    void handle_alpha();

    void handle_string();

    void handle_comment();

    void handle_multiline_comment();

    bool is_digit(const char c);

    bool is_alpha(const char c);

    char advance();

    char peek();

    char peek_next();

    bool is_at_end();

    void print_tokens();
};
