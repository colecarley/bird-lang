#pragma once
#include <variant>
#include <string>

class Value
{
public:
    using variant = std::variant<int, float, std::string, bool>;

    variant data;
    bool is_mutable;

    Value(variant data, bool is_mutable = false) : data(std::move(data)), is_mutable(is_mutable) {}
    Value() = default;
};
