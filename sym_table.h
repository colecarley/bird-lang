
#include <map>
#include <string>
#include <memory>

class SymbolTable
{
    std::map<std::string, int> vars;

public:
    void insert(std::string identifier, int value)
    {
        this->vars[identifier] = value;
    }

    int get(std::string identifier)
    {
        return this->vars[identifier];
    }
};
