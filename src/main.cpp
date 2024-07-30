#include "pugixml.hpp" 
#include "parser.hpp"
#include <iostream>
#include <string>

int main()
{
    const std::string& configFileName = "config.xml";
    parser::readFile(configFileName);
    parser::config config;
    parser::load(config);

    std::cout << config.RX_FREQ << std::endl;

    return 0;
}
