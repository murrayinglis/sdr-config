#include "pugixml.hpp" 
#include "parser.hpp"
#include <iostream>
#include <string>

int main()
{
    const std::string& configFileName = "config.xml";
    Parser parser = Parser(configFileName); // Parse config file


    return 0;
}
