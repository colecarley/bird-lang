#include "lexer.h"
#include "bird_exception.h"

void Token::print_token()
{
    std::cout << "{ "
              << "token_type: " << this->token_type
              << ",  lexeme: " << this->lexeme
              << "}"
              << std::endl;
}

std::string get_token_string(TokenType token_type)
{
    switch (token_type)
    {
    case TokenType::I32_LITERAL:
        return "I32_LITERAL";
    case TokenType::IDENTIFIER:
        return "IDENTIFIER";
    case TokenType::LET:
        return "LET";
    case TokenType::MINUS:
        return "MINUS";
    case TokenType::PLUS:
        return "PLUS";
    case TokenType::SLASH:
        return "SLASH";
    case TokenType::STAR:
        return "STAR";
    case TokenType::SEMICOLON:
        return "SEMICOLON";
    case TokenType::EQUAL:
        return "EQUAL";
    case TokenType::PUTS:
        return "PUTS";
    case TokenType::COMMA:
        return "COMMA";
    }
}

Token::Token()
{
    this->token_type = TokenType::LET;
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
    {"let", TokenType::LET}, {"puts", TokenType::PUTS}};

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
        case ' ':
        case '\n':
        case '\t':
        case '\r':
            break;
        case '+':
            this->tokens.push_back(Token(TokenType::PLUS, "+"));
            break;
        case '-':
            this->tokens.push_back(Token(TokenType::MINUS, "-"));
            break;
        case '/':
            this->tokens.push_back(Token(TokenType::SLASH, "/"));
            break;
        case '*':
            this->tokens.push_back(Token(TokenType::STAR, "*"));
            break;
        case ';':
            this->tokens.push_back(Token(TokenType::SEMICOLON, ";"));
            break;
        case '=':
            this->tokens.push_back(Token(TokenType::EQUAL, "="));
            break;
        case ',':
            this->tokens.push_back(Token(TokenType::COMMA, ","));
            break;
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
    return this->tokens;
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

    this->tokens.push_back(Token(TokenType::I32_LITERAL, number));
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

bool Lexer::is_at_end()
{
    return this->position >= this->input.length();
}
