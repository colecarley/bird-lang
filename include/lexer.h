#pragma once

#include <string>
#include <vector>
#include <iostream>
#include <map>

// forward declaration
class UserErrorTracker;

class Token
{
public:
    enum Type
    {
        // keywords
        VAR,
        PRINT,
        IF,
        ELSE,
        WHILE,
        FN,
        RETURN,
        CONST,

        // types
        TYPE_IDENTIFIER,

        // single character
        STAR,
        SLASH,
        PLUS,
        MINUS,
        SEMICOLON,
        QUESTION, // adding ternary tokens to lexer

        PERCENT,
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
        ARROW,
        PLUS_EQUAL,
        MINUS_EQUAL,
        STAR_EQUAL,
        SLASH_EQUAL,
        PERCENT_EQUAL,

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

    Type token_type;
    std::string lexeme;
    unsigned int line_num;
    unsigned int char_num;

    void print_token();

    Token();

    Token(Type token_type, std::string lexeme, unsigned int line_num, unsigned int char_num);
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
    unsigned int line_num;
    unsigned int char_num;
    std::vector<Token> tokens;
    static const std::map<std::string, Token::Type> keywords;
    UserErrorTracker *user_error_tracker;

    Lexer(std::string input, UserErrorTracker *error_tracker);

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

    void push_token(Token::Type, std::string lexeme);
};
