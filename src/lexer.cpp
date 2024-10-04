// including in the .cpp due to circular dependency
#include "../include/lexer.h"
#include "../include/exceptions/bird_exception.h"
#include "../include/exceptions/user_error_tracker.h"

static const std::map<Token::Type, std::string> token_strings = {
    {Token::Type::INT_LITERAL, "INT_LITERAL"},
    {Token::Type::IDENTIFIER, "IDENTIFIER"},
    {Token::Type::VAR, "VAR"},
    {Token::Type::CONST, "CONST"},
    {Token::Type::MINUS, "MINUS"},
    {Token::Type::PERCENT, "PERCENT"},
    {Token::Type::PLUS, "PLUS"},
    {Token::Type::SLASH, "SLASH"},
    {Token::Type::STAR, "STAR"},
    {Token::Type::SEMICOLON, "SEMICOLON"},
    {Token::Type::QUESTION, "QUESTION"},
    {Token::Type::EQUAL, "EQUAL"},
    {Token::Type::PRINT, "PRINT"},
    {Token::Type::COMMA, "COMMA"},
    {Token::Type::RBRACE, "RBRACE"},
    {Token::Type::LBRACE, "LBRACE"},
    {Token::Type::RPAREN, "RPAREN"},
    {Token::Type::LPAREN, "LPAREN"},
    {Token::Type::TYPE_IDENTIFIER, "TYPE_IDENTIFIER"},
    {Token::Type::FLOAT_LITERAL, "FLOAT_LITERAL"},
    {Token::Type::BOOL_LITERAL, "BOOL_LITERAL"},
    {Token::Type::STR_LITERAL, "STR_LITERAL"},
    {Token::Type::COLON, "COLON"},
    {Token::Type::IF, "IF"},
    {Token::Type::ELSE, "ELSE"},
    {Token::Type::WHILE, "WHILE"},
    {Token::Type::FOR, "FOR"},
    {Token::Type::GREATER, "GREATER"},
    {Token::Type::LESS, "LESS"},
    {Token::Type::BANG, "BANG"},
    {Token::Type::ARROW, "ARROW"},
    {Token::Type::GREATER_EQUAL, "GREATER_EQUAL"},
    {Token::Type::LESS_EQUAL, "LESS_EQUAL"},
    {Token::Type::EQUAL_EQUAL, "EQUAL_EQUAL"},
    {Token::Type::BANG_EQUAL, "BANG_EQUAL"},
    {Token::Type::RETURN, "RETURN"},
    {Token::Type::FN, "FN"},
    {Token::Type::END, "END"},
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
    this->token_type = Token::Type::VAR;
    this->lexeme = "";
}

Token::Token(Token::Type token_type, std::string lexeme, unsigned int line_num, unsigned int char_num)
{
    this->token_type = token_type;
    this->lexeme = lexeme;
    this->line_num = line_num;
    this->char_num = char_num;
}

Lexer::Lexer(std::string input, UserErrorTracker *user_error_tracker)
{
    this->input = input;
    this->position = 0;
    this->line_num = 0;
    this->char_num = 0;
    this->user_error_tracker = user_error_tracker;
}

const std::map<std::string, Token::Type> Lexer::keywords = {
    {"var", Token::Type::VAR},
    {"print", Token::Type::PRINT},
    {"if", Token::Type::IF},
    {"else", Token::Type::ELSE},
    {"while", Token::Type::WHILE},
    {"for", Token::Type::FOR},
    {"int", Token::Type::TYPE_IDENTIFIER},
    {"float", Token::Type::TYPE_IDENTIFIER},
    {"str", Token::Type::TYPE_IDENTIFIER},
    {"bool", Token::Type::TYPE_IDENTIFIER},
    {"void", Token::Type::TYPE_IDENTIFIER},
    {"true", Token::Type::BOOL_LITERAL},
    {"false", Token::Type::BOOL_LITERAL},
    {"fn", Token::Type::FN},
    {"return", Token::Type::RETURN},
    {"const", Token::Type::CONST},
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

            this->push_token(Token::Type::SLASH, "/");
            break;
        }
        case '\n':
        case ' ':
        case '\t':
        case '\r':
            break;
        case '+':
            this->push_token(Token::Type::PLUS, "+");
            break;
        case '-':
            if (this->peek_next() == '>')
            {
                this->advance();
                this->push_token(Token::Type::ARROW, "->");
            }
            else
            {
                this->push_token(Token::Type::MINUS, "-");
            }
            break;
        case '*':
            this->push_token(Token::Type::STAR, "*");
            break;
        case '%':
            this->push_token(Token::Type::PERCENT, "%");
            break;
        case ';':
            this->push_token(Token::Type::SEMICOLON, ";");
            break;
        case '?':
            this->push_token(Token::Type::QUESTION, "?");
            break;
        case '=':
        {
            if (this->peek_next() == '=')
            {
                this->advance();
                this->push_token(Token::Type::EQUAL_EQUAL, "==");
                break;
            }
            this->push_token(Token::Type::EQUAL, "=");
            break;
        }
        case ',':
            this->push_token(Token::Type::COMMA, ",");
            break;
        case '{':
            this->push_token(Token::Type::LBRACE, "{");
            break;
        case '}':
            this->push_token(Token::Type::RBRACE, "}");
            break;
        case '(':
            this->push_token(Token::Type::LPAREN, "(");
            break;
        case ')':
            this->push_token(Token::Type::RPAREN, ")");
            break;
        case ':':
            this->push_token(Token::Type::COLON, ":");
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
                this->push_token(Token::Type::GREATER_EQUAL, ">=");
                break;
            }
            this->push_token(Token::Type::GREATER, ">");
            break;
        }
        case '<':
        {
            if (this->peek_next() == '=')
            {
                this->advance();
                this->push_token(Token::Type::LESS_EQUAL, "<=");
                break;
            }
            this->push_token(Token::Type::LESS, "<");
            break;
        }
        case '!':
        {
            if (this->peek_next() == '=')
            {
                this->advance();
                this->push_token(Token::Type::BANG_EQUAL, "!=");
                break;
            }
            this->push_token(Token::Type::BANG, "!");
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

            this->user_error_tracker->undefined_character(c, this->line_num, this->char_num);
        }
        }
        this->advance();
    }

    this->push_token(Token::Type::END, "");
    return this->tokens;
}

void Lexer::handle_string()
{
    unsigned int start_line_num = this->line_num;
    unsigned int start_char_num = this->char_num;

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
        this->user_error_tracker->unterminated("string", start_line_num, start_char_num);
    }

    this->advance();

    this->push_token(Token::Type::STR_LITERAL, str);
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

    this->push_token(this->keywords.find(identifier) != this->keywords.end()
                         ? this->keywords.at(identifier)
                         : Token::Type::IDENTIFIER,
                     identifier);
}

/*
 * TODO: decimal
 */
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

    if (this->is_at_end())
    {
        this->push_token(Token::Type::INT_LITERAL, number);
        return;
    }

    // TODO: what happens with 42foo ?

    if (c == '.')
    {
        number.push_back(c);
        this->advance();
        c = this->peek();
        while (this->is_digit(c) && !this->is_at_end())
        {
            number.push_back(c);
            this->advance();
            c = this->peek();
        }

        this->push_token(Token::Type::FLOAT_LITERAL, number);
        return;
    }

    this->push_token(Token::Type::INT_LITERAL, number);
}

void Lexer::handle_comment()
{
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
    unsigned int start_line_num = this->line_num;
    unsigned int start_char_num = this->char_num;

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

    this->user_error_tracker->unterminated("multiline comment", start_line_num, start_char_num);
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

    if (result == '\n')
    {
        this->line_num += 1;
        this->char_num = 0;
    }
    else
    {
        this->char_num += 1;
    }

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
        this->user_error_tracker->unexpected_end_of_input();
    }

    return this->input[this->position + 1];
}

bool Lexer::is_at_end()
{
    return this->position >= this->input.length();
}

void Lexer::push_token(Token::Type token_type, std::string lexeme)
{
    this->tokens.push_back(Token(token_type, lexeme, this->line_num, this->char_num));
}