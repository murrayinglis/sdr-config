#include "config.hpp"
#include <uhd/types/device_addr.hpp>
#include <uhd/device.hpp>
#include <string>
#include <sstream>

namespace config
{
    std::string SDR_IP;
    std::string TX_SUBDEV;
    std::string RX_SUBDEV;
    std::string REF_CLOCK;
    std::string TX_ANTENNA;
    std::string RX_ANTENNA;
    std::string TEST_TYPE;
    std::string WAVEFORM_FILE;
    //
    double TX_FREQ;
    double TX_RATE;
    double TX_BW;
    double TX_GAIN;
    double RX_FREQ;
    double RX_RATE;
    double RX_BW;
    double RX_GAIN;
    //
    std::string OUTPUT_FILE;
    bool VERBOSE;
    pugi::xml_document doc;

    int readFile(const std::string& xmlFile)
    {
        
        pugi::xml_parse_result result = doc.load_file("config.xml");

        if (!result)
        {
            std::cerr << "XML parsed with errors, error description: " << result.description() << std::endl;
            return -1;
        }
        std::cout << "Load result: " << result.description() << std::endl;
        return 0;
    }

    int load()
    {
        pugi::xml_node root = doc.child("root");
        pugi::xml_node deviceNode = root.child("device");
        if (!deviceNode)
        {
            std::cerr << "Device node missing." << std::endl;
            return -1;
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
            return -1;
        }

        // Propogate settings to device
        // Device config
        pugi::xml_node configNode = root.child("config");
        TX_SUBDEV = configNode.child_value("TX_SUBDEV");
        RX_SUBDEV = configNode.child_value("RX_SUBDEV");
        REF_CLOCK = configNode.child_value("REF_CLOCK");
        TX_ANTENNA = configNode.child_value("TX_ANTENNA");
        RX_ANTENNA = configNode.child_value("RX_ANTENNA");
        TEST_TYPE = configNode.child_value("TEST_TYPE");
        WAVEFORM_FILE = configNode.child_value("WAVEFORM_FILE");
        // Frequency params
        pugi::xml_node freqNode = root.child("frequency");
        TX_FREQ = std::stod(freqNode.child_value("TX_FREQ"));
        TX_RATE = std::stod(freqNode.child_value("TX_RATE"));
        TX_BW = std::stod(freqNode.child_value("TX_BW"));
        TX_GAIN = std::stod(freqNode.child_value("TX_GAIN"));
        RX_FREQ = std::stod(freqNode.child_value("RX_FREQ"));
        RX_RATE = std::stod(freqNode.child_value("RX_RATE"));
        RX_BW = std::stod(freqNode.child_value("RX_BW"));
        RX_GAIN = std::stod(freqNode.child_value("RX_GAIN"));
        // Options
        pugi::xml_node optionsNode = root.child("options");
        OUTPUT_FILE = optionsNode.child_value("OUTPUT_FILE");
        std::stringstream ss(optionsNode.child_value("VERBOSE"));
        bool b;
        if(!(ss >> std::boolalpha >> b)) {
            std::cerr << "Verbose bool error" << std::endl;
            return -1;
        }
        VERBOSE = b;

        return 0;
    }
}




// TODO:
int checkMissingNodes()
{
    return 0;
}