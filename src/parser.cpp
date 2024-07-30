#include "parser.hpp"
#include <uhd/types/device_addr.hpp>
#include <uhd/device.hpp>
#include <string>
#include <sstream>

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
    pugi::xml_node root = doc.child("root");
    pugi::xml_node deviceNode = root.child("device");
    if (!deviceNode)
    {
        std::cerr << "Device node missing." << std::endl;
        return;
    }

    pugi::xml_node deviceName = deviceNode.child("name");
    pugi::xml_node deviceIP = deviceNode.child("IP");

    // Device to find
    std::cout << "Searching for device: " << deviceName.child_value() << ", with IP: " << deviceIP.child_value() << std::endl;

    // This is the same as running uhd_find_devices from command line
    uhd::device_addr_t hint; //an empty hint discovers all devices
    uhd::device_addrs_t dev_addrs = uhd::device::find(hint); // vector of device addresses
    // TEMP
    uhd::device_addr_t temp(std::string("addr=192.168.101.1"));
    dev_addrs.push_back(temp);
    std::cout << "Devices found: " << dev_addrs.size() << std::endl;

    // Check if desired IP is in found IPs
    // TODO: check for name? X300 etc
    uhd::device_addr_t desiredIP((std::string)("addr=") += (std::string)(deviceIP.child_value()));
    bool found;
    for (uhd::device_addr_t addr : dev_addrs)
    {
        if (desiredIP.to_string() == addr.to_string())
        {
            found = true;
            std::cout << "Device found at specified IP" << std::endl;
        }
    }
    if (!found)
    {
        std::cerr << "Device IP not found." << std::endl;
        return;
    }

    // Propogate settings to device
    // Device config
    pugi::xml_node configNode = root.child("config");
    std::string TX_SUBDEV = configNode.child_value("TX_SUBDEV");
    std::string RX_SUBDEV = configNode.child_value("RX_SUBDEV");
    std::string REF_CLOCK = configNode.child_value("REF_CLOCK");
    std::string TX_ANTENNA = configNode.child_value("TX_ANTENNA");
    std::string RX_ANTENNA = configNode.child_value("RX_ANTENNA");
    std::string TEST_TYPE = configNode.child_value("TEST_TYPE");
    std::string WAVEFORM_FILE = configNode.child_value("WAVEFORM_FILE");
    // Frequency params
    pugi::char_t a;
    pugi::xml_node freqNode = root.child("frequency");
    double TX_FREQ = std::stod(freqNode.child_value("TX_FREQ"));
    double TX_RATE = std::stod(freqNode.child_value("TX_RATE"));
    double TX_BW = std::stod(freqNode.child_value("TX_BW"));
    double TX_GAIN = std::stod(freqNode.child_value("TX_GAIN"));
    double RX_FREQ = std::stod(freqNode.child_value("RX_FREQ"));
    double RX_RATE = std::stod(freqNode.child_value("RX_RATE"));
    double RX_BW = std::stod(freqNode.child_value("RX_BW"));
    double RX_GAIN = std::stod(freqNode.child_value("RX_GAIN"));
    // Options
    pugi::xml_node optionsNode = root.child("options");
    std::string OUTPUT_FILE = optionsNode.child_value("OUTPUT_FILE");
    std::stringstream ss(optionsNode.child_value("VERBOSE"));
    bool b;
    if(!(ss >> std::boolalpha >> b)) {
        std::cerr << "Verbose bool error" << std::endl;
        return;
    }
    bool VERBOSE = b;
}

// TODO:
int checkMissingNodes()
{
    return 0;
}