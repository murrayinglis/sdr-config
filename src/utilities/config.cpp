#include "config.hpp"
#include "hardware.hpp"
#include "utils.hpp"
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
    bool VERBOSE = false;
    //
    double MIN_FREQ;
    double MAX_FREQ;

    USRP_MODE device_mode;

    uint64_t experimentZeroTime;
    
    pugi::xml_document doc;

    /**
     * This loads in a new config
     */
    int configFromFile(std::string xmlFile)
    {
        
        pugi::xml_parse_result result = doc.load_file(xmlFile.c_str());

        if (!result)
        {
            std::cerr << "XML parsed with errors, error description: " << result.description() << std::endl;
            return -1;
        }
        std::cout << "XML parse result: " << result.description() << std::endl;

        if (load() != 0)
        {
            return -1;
        }
        setUSRP_mode_from_config();
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
        SDR_IP = deviceNode.child_value("IP");


        // Propagate settings to device
        // Device config
        pugi::xml_node configNode = root.child("config");
        TEST_TYPE = configNode.child_value("TEST_TYPE");
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
        if(TX_ANTENNA=="" && RX_ANTENNA==""){ //at least one antenna is defined
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
             if (VERBOSE) std::cout<<"Device configured as RX only"<<std::endl;
            device_mode=RX_ONLY_MODE;
        }
        if(TX_ANTENNA!=""&&RX_ANTENNA==""){ // RX only mode
            if (VERBOSE) std::cout<<"Device configured as TX only"<<std::endl;
            device_mode=TX_AND_RX_MODE;
        }
        if(TX_ANTENNA!=""&&RX_ANTENNA!=""){ // RX only mode
            if (VERBOSE) std::cout<<"Device configured as TX and RX"<<std::endl;
            device_mode=TX_AND_RX_MODE;
        }
        return true;
    }

    int connect()
    {        
        // 1. Get vector of connected devices - check if there are any
        // This is the same as running uhd_find_devices from command line
        uhd::device_addr_t hint; //an empty hint discovers all devices
        uhd::device_addrs_t dev_addrs = uhd::device::find(hint); // vector of device addresses
        if (dev_addrs.size() == 0)
        {
            std::cerr << "No devices found" << std::endl;
            return -1;
        }

        // 2. Check if desired IP is in vector of connected devices
        bool found;
        for (uhd::device_addr_t addr : dev_addrs)
        {
            // keys in device_addr_t dict are: addr,name,serial
            std::cout << addr.to_string() << std::endl;
            std::string s = addr.get("addr");
            if (s == SDR_IP)
            {
                found = true;
                break;
            }
            
        }
        if (!found)
        {
            std::cerr << "Device IP not found." << std::endl;
            return -1;
        }

        // 3. Make connection
        // Setup USRP object
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
        
        // 4. Get possible parameters
        if (getPossibleParams(usrp) != 0)
        {
            return -1;
        }

        // 5. Check specified config is in possible parameters
        
        // 6. Set parameters
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

    // TODO: check for missing nodes
    int checkMissingNodes()
    {
        return 0;
    }

    int getPossibleParams(uhd::usrp::multi_usrp::sptr usrp)
    {
        bool found = false;
        int err = 0;
        // -------------------------------------------------------------------
        // RX PARAMS
        // subdevice
        // dc offset range
        // filters

        // antenna
        // TX/RX - can be set to either tx or rx
        // RX2 - set to receive
        // CAL - ?
        std::vector<std::string> rx_antennas = usrp->get_rx_antennas(0);
        for (auto it : rx_antennas)
        {
            if (RX_ANTENNA == it)
            {
                found = true;
            }
        }
        if (found == false)
        {
            std::cerr << "Cannot set RX antenna to: " << RX_ANTENNA << std::endl;
            std::cerr << "Available RX antennas: ";
            utils::ppVector(rx_antennas);
            err = -1;
        }

        // sample rate?
        uhd::meta_range_t rx_rates = usrp->get_rx_rates(0);
        if (RX_RATE > rx_rates.stop() || RX_RATE < rx_rates.start())
        {
            std::cerr << "RX rate is outside of tunable range: " << RX_RATE << std::endl;
            std::cerr << "Tunable range is from: " << rx_rates.start() << " to " << rx_rates.stop() << std::endl;
            err = -1;
        }

        // bandwidth range
        uhd::meta_range_t rx_bandwidth_range = usrp->get_rx_bandwidth_range(0);
        if (RX_BW > rx_bandwidth_range.stop())
        {
            std::cerr << "Specified RX bandwidth is too large: " << RX_BW << std::endl;
            std::cerr << "Maximum RX bandwidth: " << rx_bandwidth_range.stop() << std::endl;
            err = -1;
        }

        // freq range
        uhd::freq_range_t rx_freq_range = usrp->get_rx_freq_range(0);
        if (RX_FREQ > rx_freq_range.stop() || RX_FREQ < rx_freq_range.start())
        {
            std::cerr << "RX centre freq is outside of tunable range: " << RX_FREQ << std::endl;
            std::cerr << "Tunable range is from: " << rx_freq_range.start() << " to " << rx_freq_range.stop() << std::endl;
            err = -1;
        }

        
        // gain names
        std::vector<std::string> rx_gain_names = usrp->get_rx_gain_names(0);
        utils::ppVector(rx_gain_names);

        // gain profile names
        std::vector<std::string> rx_gain_profile_names = usrp->get_rx_gain_profile_names(0);
        utils::ppVector(rx_gain_profile_names);

        // gain range
        uhd::gain_range_t rx_gain_range = usrp->get_rx_gain_range(0);
        if (RX_GAIN > rx_gain_range.stop() || RX_GAIN < rx_gain_range.start())
        {
            std::cerr << "RX gain is outside of tunable range: " << RX_GAIN << std::endl;
            std::cerr << "Tunable range is from: " << rx_gain_range.start() << " to " << rx_gain_range.stop() << std::endl;
            err = -1;            
        }

        // LO freq range
        // LO names
        // LO sources
        // num channels
        // power range
        // sensors

        // -------------------------------------------------------------------
        // RX PARAMS
        // subdevice
        // dc offset range
        // filters

        // antenna
        // TX/RX - can be set to either tx or rx
        // CAL - ?
        std::vector<std::string> tx_antennas = usrp->get_tx_antennas(0);
        for (auto it : tx_antennas)
        {
            if (TX_ANTENNA == it)
            {
                found = true;
            }
        }
        if (found == false)
        {
            std::cerr << "Cannot set TX antenna to: " << TX_ANTENNA << std::endl;
            std::cerr << "Available TX antennas: ";
            utils::ppVector(tx_antennas);
            err = -1;
        }

        // sample rate?
        uhd::meta_range_t tx_rates = usrp->get_tx_rates(0);
        if (TX_RATE > tx_rates.stop() || TX_RATE < tx_rates.start())
        {
            std::cerr << "TX rate is outside of tunable range: " << TX_RATE << std::endl;
            std::cerr << "Tunable range is from: " << tx_rates.start() << " to " << tx_rates.stop() << std::endl;
            err = -1;
        }

        // bandwidth range
        uhd::meta_range_t tx_bandwidth_range = usrp->get_tx_bandwidth_range(0);
        if (TX_BW > tx_bandwidth_range.stop())
        {
            std::cerr << "Specified TX bandwidth is too large: " << TX_BW << std::endl;
            std::cerr << "Maximum TX bandwidth: " << tx_bandwidth_range.stop() << std::endl;
            err = -1;
        }

        // freq range
        uhd::freq_range_t tx_freq_range = usrp->get_tx_freq_range(0);
        if (TX_FREQ > tx_freq_range.stop() || TX_FREQ < tx_freq_range.start())
        {
            std::cerr << "TX centre freq is outside of tunable range: " << TX_FREQ << std::endl;
            std::cerr << "Tunable range is from: " << tx_freq_range.start() << " to " << tx_freq_range.stop() << std::endl;
            err = -1;
        }

        
        // gain names
        std::vector<std::string> tx_gain_names = usrp->get_tx_gain_names(0);
        utils::ppVector(tx_gain_names);

        // gain profile names
        std::vector<std::string> tx_gain_profile_names = usrp->get_tx_gain_profile_names(0);
        utils::ppVector(tx_gain_profile_names);

        // gain range
        uhd::gain_range_t tx_gain_range = usrp->get_tx_gain_range(0);
        if (TX_GAIN > tx_gain_range.stop() || TX_GAIN < tx_gain_range.start())
        {
            std::cerr << "TX gain is outside of tunable range: " << TX_GAIN << std::endl;
            std::cerr << "Tunable range is from: " << tx_gain_range.start() << " to " << tx_gain_range.stop() << std::endl;
            err = -1;            
        }

        // LO freq range
        // LO names
        // LO sources
        // num channels
        // power range
        // sensors

        // time sources
        // sync sources
        return err;
    }
}

