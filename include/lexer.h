#pragma once

#include <string>
#include <vector>
#include <iostream>
#include <map>

enum TokenType
{
    // keywords
    LET,
    PUTS,

    // single character
    STAR,
    SLASH,
    PLUS,
    MINUS,
    SEMICOLON,
    EQUAL,
    COMMA,

    // values
    IDENTIFIER,
    I32_LITERAL,
};

std::string get_token_string(TokenType);

class Token
{
public:
    TokenType token_type;
    std::string lexeme;

    void print_token();

    Token();

    Token(TokenType token_type, std::string lexeme);
};

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

    bool is_digit(const char c);

    bool is_alpha(const char c);

    char advance();

    char peek();

    bool is_at_end();

    void print_tokens();
};
