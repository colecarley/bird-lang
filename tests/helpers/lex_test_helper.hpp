#include <vector>
#include "../../src/lexer.cpp"
#include "../../include/exceptions/user_error_tracker.h"
#include "../../include/lexer.h"

std::vector<Token> lex_code(const std::string &code)
{
    UserErrorTracker error_tracker(code);
    Lexer lexer(code, &error_tracker);
    return lexer.lex();
}