#include "pugixml.hpp" 
#include "config.hpp"
#include <iostream>
#include <string>

int main()
{
    const std::string& configFileName = "config.xml";
    config::readFile(configFileName);
    config::load();


    return 0;
}
