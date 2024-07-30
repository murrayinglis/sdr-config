#include "pugixml.hpp" 
#include "parser.hpp"
#include <iostream>
#include <string>

int main()
{
    const std::string& configFileName = "config.xml";
    parser::readFile(configFileName);
    parser::load();


    return 0;
}
