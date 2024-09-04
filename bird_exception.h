#pragma once

#include <exception>
#include <string>

class BirdException : public std::exception
{
private:
    std::string message;

public:
    BirdException(const std::string &msg) : message(msg) {}

    const char *what() const noexcept override
    {
        return message.c_str();
    }
};