#pragma once

#include <vector>

enum class BirdType
{
    INT,
    FLOAT,
    STRING,
    BOOL,
    VOID,
};

// TODO: figure out how to do first class functions
struct BirdFunction
{
    std::vector<BirdType> params;
    BirdType ret;

    BirdFunction(std::vector<BirdType> params, BirdType ret) : params(params), ret(ret) {}
    BirdFunction() = default;
};
