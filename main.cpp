#include "build/parser.tab.hh"
#include "src/scanner.hpp"
#include <fstream>
#include <iostream>

int main(int argc, char *argv[])
{
    std::ifstream file("../code.bird");
    if (!file.is_open())
    {
        std::cout << "failed to open file" << std::endl;
        return -1;
    }
    Bird::Scanner scanner(&file);
    Bird::Parser parser(scanner);
    // parser.set_debug_level(1);
    parser.parse();
}