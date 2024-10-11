#pragma once
#include <variant>
#include <string>

using variant = std::variant<int, float, std::string, bool>;
class Value
{
public:
    variant data;
    bool is_mutable;

    Value(variant data, bool is_mutable = false) : data(std::move(data)), is_mutable(is_mutable) {}
    Value() = default;
};

template <typename T>
inline bool is_type(Value value)
{
    return std::holds_alternative<T>(value.data);
}

inline bool is_numeric(Value value)
{
    return is_type<int>(value) || is_type<float>(value);
}

template <typename T>
inline bool is_matching_type(Value left, Value right)
{
    return is_type<T>(left) && is_type<T>(right);
}

template <typename T>
inline T as_type(Value value)
{
    return std::get<T>(value.data);
}

template <typename T, typename U>
inline T to_type(Value value)
{
    return is_type<T>(value) ? as_type<T>(value) : static_cast<T>(as_type<U>(value));
}
