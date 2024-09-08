#pragma once

#include <map>
#include <string>
#include <memory>

template <typename T>
class SymbolTable
{
    std::map<std::string, T> vars;

public:
    void insert(std::string identifier, T value)
    {
        this->vars[identifier] = value;
    }

    T get(std::string identifier)
    {
        return this->vars[identifier];
    }
};
