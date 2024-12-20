#pragma once

#include "lexer.h"

class Type
{
public:
    Token type;

    Type() = default;
    Type(Token typeP) : type(typeP) {}
};

struct SemanticType
{
    SemanticType() = default;
};