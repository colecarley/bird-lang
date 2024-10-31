#pragma once

#include <exception>
#include <string>

/*
 * Continue exception that should be thrown when a continue statement is present in a loop,
 * which results in the termination of the current loop frame to continue to the next loop frame.
 */
class ContinueException : public std::exception
{
private:
public:
    ContinueException() {}
};