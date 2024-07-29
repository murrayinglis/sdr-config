#include "pugixml.hpp" 
#include <iostream>

int main()
{
    pugi::xml_document doc;

    pugi::xml_parse_result result = doc.load_file("config.xml");

    if (!result) {
        std::cerr << "XML parsed with errors, error description: " << result.description() << std::endl;
        return 1;
    }

    pugi::xml_node device = doc.child("device");
    if (!device) {
        std::cerr << "No <device> element found in config.xml" << std::endl;
        return 1;
    }

    pugi::xml_node name_node = device.child("name");
    if (!name_node) {
        std::cerr << "No <name> element found inside <device>" << std::endl;
        return 1;
    }

    std::cout << "Load result: " << result.description() << std::endl;
    std::cout << "Device name: " << name_node.child_value() << std::endl;
    return 0;
}
