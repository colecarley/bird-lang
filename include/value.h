#pragma once
#include <variant>
#include <string>

#include "exceptions/bird_exception.h"

class Value;

template <typename T>
inline bool is_type(Value value);

inline bool is_numeric(Value value);

template <typename T>
inline bool is_matching_type(Value left, Value right);

template <typename T>
inline T as_type(Value value);

template <typename T, typename U>
inline T to_type(Value value);

using variant = std::variant<int, double, std::string, bool>;
class Value
{
public:
    variant data;
    bool is_mutable;

    Value(variant data, bool is_mutable = false) : data(std::move(data)), is_mutable(is_mutable) {}
    Value() = default;

    Value operator+(Value right)
    {
        if (is_matching_type<std::string>(*this, right))
            return Value(as_type<std::string>(*this) + as_type<std::string>(right));

        if (is_type<int>(*this) && is_numeric(right))
            return Value(as_type<int>(*this) + to_type<int, double>(right));

        if (is_type<double>(*this) && is_numeric(right))
            return Value(as_type<double>(*this) + to_type<double, int>(right));

        throw BirdException("The '+' binary operator could not be used to interpret these values.");
    }

    Value operator-(Value right)
    {
        if (is_type<int>(*this) && is_numeric(right))
            return Value(as_type<int>(*this) - to_type<int, double>(right));

        if (is_type<double>(*this) && is_numeric(right))
            return Value(as_type<double>(*this) - to_type<double, int>(right));

        throw BirdException("The '-' binary operator could not be used to interpret these values.");
    }

    Value operator*(Value right)
    {
        if (is_type<int>(*this) && is_numeric(right))
            return Value(as_type<int>(*this) * to_type<int, double>(right));

        if (is_type<double>(*this) && is_numeric(right))
            return Value(as_type<double>(*this) * to_type<double, int>(right));

        throw BirdException("The '*' binary operator could not be used to interpret these values.");
    }

    Value operator/(Value right)
    {
        if (is_type<int>(right) && as_type<int>(right) == 0)
            throw BirdException("Division by zero.");

        if (is_type<double>(right) && as_type<double>(right) == 0)
            throw BirdException("Division by zero.");

        if (is_type<int>(*this) && is_numeric(right))
            return Value(as_type<int>(*this) / to_type<int, double>(right));

        if (is_type<double>(*this) && is_numeric(right))
            return Value(as_type<double>(*this) / to_type<double, int>(right));

        throw BirdException("The '/' binary operator could not be used to interpret these values.");
    }

    Value operator%(Value right)
    {
        if (is_type<int>(right) && as_type<int>(right) == 0)
            throw BirdException("Modulo by zero.");

        if (is_type<double>(right) && as_type<double>(right) == 0)
            throw BirdException("Modulo by zero.");

        if (is_type<int>(*this) && is_numeric(right))
            return Value(as_type<int>(*this) % to_type<int, double>(right));

        throw BirdException("The '%' binary operator could not be used to interpret these values.");
    }

    Value operator>(Value right)
    {
        if (is_type<int>(*this) && is_numeric(right))
            return Value(as_type<int>(*this) > to_type<int, double>(right));

        if (is_type<double>(*this) && is_numeric(right))
            return Value(as_type<double>(*this) > to_type<double, int>(right));

        throw BirdException("The '>' binary operator could not be used to interpret these values.");
    }

    Value operator>=(Value right)
    {
        if (is_type<int>(*this) && is_numeric(right))
            return Value(as_type<int>(*this) >= to_type<int, double>(right));

        if (is_type<double>(*this) && is_numeric(right))
            return Value(as_type<double>(*this) >= to_type<double, int>(right));

        throw BirdException("The '>=' binary operator could not be used to interpret these values.");
    }

    Value operator<(Value right)
    {
        if (is_type<int>(*this) && is_numeric(right))
            return Value(as_type<int>(*this) < to_type<int, double>(right));

        if (is_type<double>(*this) && is_numeric(right))
            return Value(as_type<double>(*this) < to_type<double, int>(right));

        throw BirdException("The '<' binary operator could not be used to interpret these values.");
    }

    Value operator<=(Value right)
    {

        if (is_type<int>(*this) && is_numeric(right))
            return Value(as_type<int>(*this) <= to_type<int, double>(right));

        if (is_type<double>(*this) && is_numeric(right))
            return Value(as_type<double>(*this) <= to_type<double, int>(right));

        throw BirdException("The '<=' binary operator could not be used to interpret these values.");
    }

    Value operator!=(Value right)
    {
        if (is_numeric(*this) && is_numeric(right))
        {
            double left_double = is_type<int>(*this) ? as_type<int>(*this) : as_type<double>(*this);
            double right_double = is_type<int>(right) ? as_type<int>(right) : as_type<double>(right);

            return Value(left_double != right_double);
        }

        if (is_type<std::string>(*this) && is_type<std::string>(right))
            return Value(as_type<std::string>(*this) != as_type<std::string>(right));

        if (is_type<bool>(*this) && is_type<bool>(right))
            return Value(as_type<bool>(*this) != as_type<bool>(right));

        throw BirdException("The '!=' binary operator could not be used to interpret these values.");
    }

    Value operator==(Value right)
    {
        if (is_numeric(*this) && is_numeric(right))
        {
            double left_double = is_type<int>(*this) ? as_type<int>(*this) : as_type<double>(*this);
            double right_double = is_type<int>(right) ? as_type<int>(right) : as_type<double>(right);

            return Value(left_double == right_double);
        }

        if (is_type<std::string>(*this) && is_type<std::string>(right))
            return Value(as_type<std::string>(*this) == as_type<std::string>(right));

        if (is_type<bool>(*this) && is_type<bool>(right))
            return Value(as_type<bool>(*this) == as_type<bool>(right));

        throw BirdException("The '==' binary operator could not be used to interpret these values.");
    }

    Value operator!()
    {
        if (is_type<bool>(*this))
            return Value(!as_type<bool>(*this));

        throw BirdException("The '!' unary operator could not be used to interpret these values.");
    }

    Value operator-()
    {
        if (is_type<int>(*this))
            return Value(-as_type<int>(*this));

        if (is_type<double>(*this))
            return Value(-as_type<double>(*this));

        throw BirdException("The '-' unary operator could not be used to interpret these values.");
    }

    Value &operator=(const Value &right)
    {
        if (this != &right)
        {
            this->data = right.data;
            this->is_mutable = this->is_mutable ? true : right.is_mutable;
        }

        return *this;
    }

    friend std::ostream &operator<<(std::ostream &os, const Value &obj)
    {
        if (is_type<int>(obj))
            os << as_type<int>(obj);

        else if (is_type<double>(obj))
            os << as_type<double>(obj);

        else if (is_type<std::string>(obj))
            os << as_type<std::string>(obj);

        else if (is_type<bool>(obj))
            os << as_type<bool>(obj);

        return os;
    }
};

template <typename T>
inline bool is_type(Value value)
{
    return std::holds_alternative<T>(value.data);
}

inline bool is_numeric(Value value)
{
    return is_type<int>(value) || is_type<double>(value);
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

struct SemanticValue
{
    bool is_mutable;

    SemanticValue(bool is_mutable) : is_mutable(is_mutable) {}
    SemanticValue() = default;
};