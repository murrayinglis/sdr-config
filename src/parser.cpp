#include "parser.hpp"
#include <uhd/types/device_addr.hpp>
#include <uhd/device.hpp>

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

void Parser::parse()
{
    uhd::device_addr_t hint; //an empty hint discovers all devices
    uhd::device_addrs_t dev_addrs = uhd::device::find(hint);
    std::cout << "Devices found: " << dev_addrs.size() << std::endl;
}