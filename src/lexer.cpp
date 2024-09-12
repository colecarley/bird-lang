// including in the .cpp due to circular dependency
#include "../include/lexer.h"
#include "../include/exceptions/bird_exception.h"

static const std::map<TokenType, std::string> token_strings = {
    {TokenType::INT_LITERAL, "INT_LITERAL"},
    {TokenType::IDENTIFIER, "IDENTIFIER"},
    {TokenType::VAR, "VAR"},
    {TokenType::MINUS, "MINUS"},
    {TokenType::PLUS, "PLUS"},
    {TokenType::SLASH, "SLASH"},
    {TokenType::STAR, "STAR"},
    {TokenType::SEMICOLON, "SEMICOLON"},
    {TokenType::EQUAL, "EQUAL"},
    {TokenType::PRINT, "PRINT"},
    {TokenType::COMMA, "COMMA"},
    {TokenType::RBRACE, "RBRACE"},
    {TokenType::LBRACE, "LBRACE"},
    {TokenType::RPAREN, "RPAREN"},
    {TokenType::LPAREN, "LPAREN"},
    {TokenType::INT, "INT"},
    {TokenType::FLOAT, "FLOAT"},
    {TokenType::STR, "STR"},
    {TokenType::BOOL, "BOOL"},
    {TokenType::FLOAT_LITERAL, "FLOAT_LITERAL"},
    {TokenType::BOOL_LITERAL, "BOOL_LITERAL"},
    {TokenType::STR_LITERAL, "STR_LITERAL"},
    {TokenType::COLON, "COLON"},
    {TokenType::IF, "IF"},
    {TokenType::ELSE, "ELSE"},
    {TokenType::WHILE, "WHILE"},
    {TokenType::GREATER, "GREATER"},
    {TokenType::LESS, "LESS"},
    {TokenType::BANG, "BANG"},
    {TokenType::GREATER_EQUAL, "GREATER_EQUAL"},
    {TokenType::LESS_EQUAL, "LESS_EQUAL"},
    {TokenType::EQUAL_EQUAL, "EQUAL_EQUAL"},
    {TokenType::BANG_EQUAL, "BANG_EQUAL"},
};

void Token::print_token()
{
    std::cout << "{ "
              << "token_type: " << token_strings.at(this->token_type)
              << ",  lexeme: " << this->lexeme
              << "}"
              << std::endl;
}

Token::Token()
{
    this->token_type = TokenType::VAR;
    this->lexeme = "";
}

Token::Token(TokenType token_type, std::string lexeme)
{
    this->token_type = token_type;
    this->lexeme = lexeme;
}

Lexer::Lexer(std::string input)
{
    this->input = input;
    this->position = 0;
}

const std::map<std::string, TokenType> Lexer::keywords = {
    {"var", TokenType::VAR},
    {"print", TokenType::PRINT},
    {"if", TokenType::IF},
    {"else", TokenType::ELSE},
    {"while", TokenType::WHILE},
    {"int", TokenType::INT},
    {"float", TokenType::FLOAT},
    {"str", TokenType::STR},
    {"bool", TokenType::BOOL},
    {"true", TokenType::BOOL_LITERAL},
    {"false", TokenType::BOOL_LITERAL},
};

void Lexer::print_tokens()
{
    for (auto token : this->tokens)
    {
        token.print_token();
    }
}

std::vector<Token> Lexer::lex()
{
    while (!this->is_at_end())
    {
        const char c = this->peek();
        switch (c)
        {
        case '/':
        {
            if (this->peek_next() == '/')
            {
                this->handle_comment();
                continue;
            }
            if (this->peek_next() == '*')
            {
                this->handle_multiline_comment();
                continue;
            }

            this->tokens.push_back(Token(TokenType::SLASH, "/"));
            break;
        }
        case ' ':
        case '\n': // TODO: add line counter for errors
        case '\t':
        case '\r':
            break;
        case '+':
            this->tokens.push_back(Token(TokenType::PLUS, "+"));
            break;
        case '-':
            this->tokens.push_back(Token(TokenType::MINUS, "-"));
            break;
        case '*':
            this->tokens.push_back(Token(TokenType::STAR, "*"));
            break;
        case ';':
            this->tokens.push_back(Token(TokenType::SEMICOLON, ";"));
            break;
        case '=':
        {
            if (this->peek_next() == '=')
            {
                this->advance();
                this->tokens.push_back(Token(TokenType::EQUAL_EQUAL, "=="));
                break;
            }
            this->tokens.push_back(Token(TokenType::EQUAL, "="));
            break;
        }
        case ',':
            this->tokens.push_back(Token(TokenType::COMMA, ","));
            break;
        case '{':
            this->tokens.push_back(Token(TokenType::LBRACE, "{"));
            break;
        case '}':
            this->tokens.push_back(Token(TokenType::RBRACE, "}"));
            break;
        case '(':
            this->tokens.push_back(Token(TokenType::LPAREN, "("));
            break;
        case ')':
            this->tokens.push_back(Token(TokenType::RPAREN, ")"));
            break;
        case ':':
            this->tokens.push_back(Token(TokenType::COLON, ":"));
            break;
        case '"':
        {

            this->handle_string();
            continue;
        }
        case '>':
        {
            if (this->peek_next() == '=')
            {
                this->advance();
                this->tokens.push_back(Token(TokenType::GREATER_EQUAL, ">="));
                break;
            }
            this->tokens.push_back(Token(TokenType::GREATER, ">"));
            break;
        }
        case '<':
        {
            if (this->peek_next() == '=')
            {
                this->advance();
                this->tokens.push_back(Token(TokenType::LESS_EQUAL, "<="));
                break;
            }
            this->tokens.push_back(Token(TokenType::LESS, "<"));
            break;
        }
        case '!':
        {
            if (this->peek_next() == '=')
            {
                this->advance();
                this->tokens.push_back(Token(TokenType::BANG_EQUAL, "!="));
                break;
            }
            this->tokens.push_back(Token(TokenType::BANG, "!"));
            break;
        }
        default:
        {
            if (this->is_alpha(c))
            {
                this->handle_alpha();
                continue;
            }
            if (this->is_digit(c))
            {
                this->handle_number();
                continue;
            }

            throw BirdException(std::string("undefined character ") + c);
        }
        }
        this->advance();
    }

    this->tokens.push_back(Token(TokenType::END, ""));
    return this->tokens;
}

void Lexer::handle_string()
{
    this->advance();
    char c = this->peek();

    std::string str = "";
    while (c != '"' && !this->is_at_end())
    {
        str.push_back(c);
        this->advance();
        c = this->peek();
    }

    if (this->is_at_end())
    {
        throw BirdException(std::string("unterminated string"));
    }

    this->advance();

    this->tokens.push_back(Token(TokenType::STR_LITERAL, str));
}

void Lexer::handle_alpha()
{
    std::string identifier = "";
    identifier.push_back(this->advance());
    char c = this->peek();

    while ((this->is_digit(c) || this->is_alpha(c)) && !this->is_at_end())
    {
        identifier.push_back(c);
        this->advance();
        c = this->peek();
    }

    this->tokens.push_back(Token(this->keywords.find(identifier) != this->keywords.end()
                                     ? this->keywords.at(identifier)
                                     : TokenType::IDENTIFIER,
                                 identifier));
}

void Lexer::handle_number()
{
    std::string number = "";
    char c = this->peek();
    while (this->is_digit(c) && !this->is_at_end())
    {
        number.push_back(c);
        this->advance();
        c = this->peek();
    }

    if (!this->is_at_end() && c == '.')
    {
        number.push_back(c);
        this->advance();
        while (this->is_digit(c) && !this->is_at_end())
        {
            number.push_back(c);
            this->advance();
            c = this->peek();
        }

        this->tokens.push_back(Token(TokenType::FLOAT_LITERAL, number));
    }

    this->tokens.push_back(Token(TokenType::INT_LITERAL, number));
}

void Lexer::handle_comment()
{
    // skip the first two slashes '//'
    this->advance();
    this->advance();

    while (!this->is_at_end())
    {
        char c = this->advance();
        if (c == '\n')
        {
            return;
        }
    }
}

void Lexer::handle_multiline_comment()
{
    // skip the first slash star '/*'
    while (!this->is_at_end())
    {
        if (this->advance() == '*')
        {
            if (this->peek() == '/')
            {
                this->advance();
                return;
            }
        }
    }

    throw BirdException("Unterminated multiline comment");
}

bool Lexer::is_alpha(const char c)
{
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

bool Lexer::is_digit(const char c)
{
    return '0' <= c && c <= '9';
}

char Lexer::advance()
{
    auto result = this->input[this->position];
    this->position++;
    return result;
}

char Lexer::peek()
{
    return this->input[this->position];
}

char Lexer::peek_next()
{
    if (this->position + 1 >= this->input.length())
    {
        throw BirdException(std::string("Unexpected end of input"));
    }

    return this->input[this->position + 1];
}

bool Lexer::is_at_end()
{
    return this->position >= this->input.length();
}
