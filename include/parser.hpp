#ifndef PARSER_HPP
#define PARSER_HPP

#include "pugixml.hpp" 
#include <string>
#include <iostream>
#include <vector>

class Parser
{
public:
    Parser();
    Parser(const std::string& xmlFile);
    static void parse(Parser parser);

private:
    pugi::xml_document doc;
    pugi::xml_node device;
    pugi::xml_node name;

};

#endif