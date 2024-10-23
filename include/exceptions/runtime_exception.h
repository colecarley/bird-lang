#pragma once

#include <exception>
#include <string>

/*
 * Exceptions that should be thrown runtime errors, during interpretation.
 */
class RuntimeException : public std::exception
{
private:
    std::string message;

public:
    RuntimeException(const std::string &msg) : message(msg) {}

    const char *what() const noexcept override
    {
        return message.c_str();
    }
};