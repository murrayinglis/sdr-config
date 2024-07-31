#include "config.hpp"
#include "hardware.hpp"
#include <uhd/types/device_addr.hpp>
#include <uhd/device.hpp>
#include <uhd/utils/thread.hpp>
#include <uhd/utils/safe_main.hpp>
#include <uhd/usrp/multi_usrp.hpp>
#include <uhd/exception.hpp>
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
    //
    double MIN_FREQ;
    double MAX_FREQ;

    USRP_MODE device_mode;
    pugi::xml_document doc;

    int configFromFile(const std::string& xmlFile)
    {
        
        pugi::xml_parse_result result = doc.load_file("config.xml");

        if (!result)
        {
            std::cerr << "XML parsed with errors, error description: " << result.description() << std::endl;
            return -1;
        }
        std::cout << "Load result: " << result.description() << std::endl;

        load();
        setUSRP_mode_from_config();
        connect();

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
        pugi::xml_node SDR_IP = deviceNode.child("IP");

        // Device to find
        std::cout << "Searching for device: " << deviceName.child_value() << ", with IP: " << SDR_IP.child_value() << std::endl;

        // This is the same as running uhd_find_devices from command line
        uhd::device_addr_t hint; //an empty hint discovers all devices
        uhd::device_addrs_t dev_addrs = uhd::device::find(hint); // vector of device addresses
        // TEMP
        uhd::device_addr_t temp(std::string("addr=192.168.101.1"));
        dev_addrs.push_back(temp);
        std::cout << "Devices found: " << dev_addrs.size() << std::endl;

        // Check if desired IP is in found IPs
        // TODO: check for name? X300 etc
        uhd::device_addr_t desiredIP((std::string)("addr=") += (std::string)(SDR_IP.child_value()));
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

    bool setUSRP_mode_from_config()
    {
        // check for invalid modes
        if(TX_ANTENNA==""&&RX_ANTENNA==""){ //at least one antenna is defined
            std::cerr<<"No antennas specified"<<std::endl;
            device_mode=INVALID_MODE;
            return false;
        }
        if(TX_ANTENNA==RX_ANTENNA){
            std::cerr<<"Config is attempting to TX and RX on same channel:"<<std::endl;
            std::cerr<<TX_ANTENNA<<"&"<<RX_ANTENNA<<std::endl;
            device_mode=INVALID_MODE;
            return false;    
        }

        // check for valid modes
        if(TX_ANTENNA==""&&RX_ANTENNA!=""){ // RX only mode
            std::cout<<"Device configured as RX only"<<std::endl;
            device_mode=RX_ONLY_MODE;
        }
        if(TX_ANTENNA!=""&&RX_ANTENNA==""){ // RX only mode
            std::cout<<"Device configured as TX only"<<std::endl;
            device_mode=TX_AND_RX_MODE;
        }
        if(TX_ANTENNA!=""&&RX_ANTENNA!=""){ // RX only mode
            std::cout<<"Device configured as TX and RX"<<std::endl;
            device_mode=TX_AND_RX_MODE;
        }
        return true;
    }

    int connect()
    {
        uhd::usrp::multi_usrp::sptr usrp;
        try
        {
            usrp = uhd::usrp::multi_usrp::make(std::string("addr=") += config::SDR_IP);
        }
        catch(const std::exception& e)
        {
            std::cerr << e.what() << '\n';
            return -1;
        }
        
        
        std::cout << "Default (current) clock source: " << usrp->get_clock_source(0) << "\n";
        std::cout << "Default (current) time source: " << usrp->get_time_source(0) << "\n";
        // usrp->set_clock_source(CONFIG::REF_CLOCK);
        // usrp->set_time_source(CONFIG::REF_CLOCK);
        usrp->set_time_now(uhd::time_spec_t(0.0)); //! Need to remove once I have proper global timing set up
        //uint64_t zero_time_ms = UTIL::getCurrentEpochTime_ms();

        switch (config::device_mode)
        {
        case config::USRP_MODE::TX_ONLY_MODE:
            hardware::setupTransmitter(usrp);
            break;
        case config::USRP_MODE::RX_ONLY_MODE:
            hardware::setupReceiever(usrp);
            break;
        case config::USRP_MODE::TX_AND_RX_MODE:
            hardware::setupTransmitter(usrp);
            hardware::setupReceiever(usrp);
            break;
        default:
            std::cout << "Hit default case, mode not set correctly" << std::endl;
            break;
        }
        return 0;
    }

    // TODO:
    int checkMissingNodes()
    {
        return 0;
    }
}

