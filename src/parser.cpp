#include "parser.hpp"
#include <uhd/types/device_addr.hpp>
#include <uhd/device.hpp>
#include <string>

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
    pugi::xml_node deviceNode = doc.child("device");
    if (!deviceNode)
    {
        std::cerr << "Device node missing." << std::endl;
        return;
    }
    pugi::xml_node deviceName = deviceNode.child("name");
    pugi::xml_node deviceID = deviceNode.child("id");

    // Identify device
    std::cout << "Searching for device: " << deviceName.child_value() << ", with ID: " << deviceID.child_value() << std::endl;
    uhd::device_addr_t hint; //an empty hint discovers all devices
    uhd::device_addrs_t dev_addrs = uhd::device::find(hint); // vector of device addresses
    std::cout << "Devices found: " << dev_addrs.size() << std::endl;
    uhd::device_addr_t desiredDevice((std::string)(deviceName.child_value()));

    // Propogate settings to device
    pugi::xml_node configNode = doc.child("config");
    pugi::xml_node centreFrequency = configNode.child("centre_frequency");
    pugi::xml_node sampleRate = configNode.child("sample_rate");
    pugi::xml_node gain = configNode.child("gain");
    pugi::xml_node waveformPath = configNode.child("waveform_path");
}