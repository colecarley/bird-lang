#pragma once

#include <map>
#include <string>
#include <memory>

/*
 * A table with key value pairs of identifiers and their respective values
 */
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
