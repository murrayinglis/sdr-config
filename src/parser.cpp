#include "parser.hpp"

Parser::Parser()
{
    std::cout<<"No xml file specified, using default config.xml"<<std::endl;
    Parser("config.xml");
}

Parser::Parser(const std::string& xmlFile)
{
    pugi::xml_parse_result result = doc.load_file("config.xml");

    if (!result) {
        std::cerr << "XML parsed with errors, error description: " << result.description() << std::endl;
        return;
    }

    std::cout << "Load result: " << result.description() << std::endl;
}

void Parser::parse(Parser parse)
{
    
}