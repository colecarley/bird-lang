#pragma once

#include <map>
#include <string>
#include <memory>
#include "visitors/interpreter.h"
#include "exceptions/bird_exception.h"

/*
 * A table with key value pairs of identifiers and their respective values
 */
template <typename T>
class SymbolTable
{
    std::map<std::string, T> vars;
    std::unique_ptr<SymbolTable> enclosing;

public:
    void insert(std::string identifier, T value)
    {
        this->vars[identifier] = std::move(value);
    }

    T get(std::string identifier)
    {
        if (this->contains(identifier))
        {
            return std::move(this->vars[identifier]);
        }
        else
        {
            if (this->enclosing.get() == nullptr)
            {
                throw BirdException("undefined identifier");
            }
            return std::move(this->enclosing->get(identifier));
        }
    }

    std::unique_ptr<SymbolTable> get_enclosing()
    {
        return std::move(this->enclosing);
    }

    void set_enclosing(std::unique_ptr<SymbolTable> enclosing)
    {
        this->enclosing = std::move(enclosing);
    }

    bool contains(std::string identifier)
    {
        return vars.find(identifier) != vars.end();
    }
};
