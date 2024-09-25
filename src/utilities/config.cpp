#include "tests.hpp"
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
    USRP_MODE device_mode;

    uint64_t experimentZeroTime;
    pugi::xml_document doc;

    usrp_config::usrp_config() {}

    /**
     * This loads in a new config to 
     */
    int usrp_config::configFromFile(std::string xmlFile)
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

    // Load a usrp_config object with 
    int usrp_config::load()
    {
        root = doc.child("root");
        pugi::xml_node deviceNode = root.child("device");
        if (!deviceNode)
        {
            std::cerr << "Device node missing." << std::endl;
            return -1;
        }

        pugi::xml_node deviceName = deviceNode.child("name");
        SDR_IP = deviceNode.child_value("IP");


        // Propagate settings to device
        // TODO: check for missing nodes
        // IDEA: create list of present nodes to be checked when checking params
        // Device config
        // Use a lambda function to safely get the value or set to a default value if missing
        auto get_node_value = [](const pugi::xml_node& node, const std::string& name, const std::string& default_value = "") {
            const pugi::xml_node child = node.child(name.c_str());
            std::string value_str = child ? child.child_value() : default_value;
            try {
                return value_str.empty() ? default_value : value_str;
            } catch (const std::invalid_argument&) {
                std::cerr << name << " is empty." << std::endl;
                return default_value;
            } catch (const std::out_of_range&) {
                std::cerr << name << " is empty." << std::endl;
                return default_value;
            }
        };
        auto get_node_value_as_bool = [](const pugi::xml_node& node, const std::string& name, bool default_value = false) {
            const pugi::xml_node child = node.child(name.c_str());
            std::string lowerStr = child.child_value();
            // Convert string to lowercase for case-insensitive comparison
            std::transform(lowerStr.begin(), lowerStr.end(), lowerStr.begin(), ::tolower);
            
            if (lowerStr == "true" || lowerStr == "1") {
                return true;
            } else if (lowerStr == "false" || lowerStr == "0") {
                return false;
            } else {
                throw std::invalid_argument("Invalid boolean string");
            }
        };
        auto get_node_value_as_double = [](const pugi::xml_node& node, const std::string& name, double default_value = 0.0f) {
            const pugi::xml_node child = node.child(name.c_str());
            std::string value_str = child ? child.child_value() : "";
            try {
                return value_str.empty() ? default_value : std::stod(value_str);
            } catch (const std::invalid_argument&) {
                std::cerr << name << " is empty." << std::endl;
                return default_value;
            } catch (const std::out_of_range&) {
                std::cerr << name << " is empty." << std::endl;
                return default_value;
            }
        };
        pugi::xml_node configNode = root.child("config");
        TX_SUBDEV = get_node_value(configNode, "TX_SUBDEV");
        RX_SUBDEV = get_node_value(configNode, "RX_SUBDEV");
        REF_CLOCK = get_node_value(configNode, "REF_CLOCK");
        TX_ANTENNA = get_node_value(configNode, "TX_ANTENNA");
        RX_ANTENNA = get_node_value(configNode, "RX_ANTENNA");
        WAVEFORM_FILE = get_node_value(configNode, "WAVEFORM_FILE");
        // Frequency params
        pugi::xml_node freqNode = root.child("frequency");
        TX_FREQ = get_node_value_as_double(freqNode, "TX_FREQ");
        TX_RATE = get_node_value_as_double(freqNode, "TX_RATE");
        TX_BW = get_node_value_as_double(freqNode, "TX_BW");
        TX_GAIN = get_node_value_as_double(freqNode, "TX_GAIN");
        TX_DC_OFFSET = get_node_value_as_double(freqNode, "TX_DC_OFFSET");
        RX_FREQ = get_node_value_as_double(freqNode, "RX_FREQ");
        RX_RATE = get_node_value_as_double(freqNode, "RX_RATE");
        RX_BW = get_node_value_as_double(freqNode, "RX_BW");
        RX_GAIN = get_node_value_as_double(freqNode, "RX_GAIN");
        RX_DC_OFFSET = get_node_value_as_double(freqNode, "RX_DC_OFFSET");
        // Radar
        pugi::xml_node radarNode = root.child("radar");
        RADAR_TYPE = get_node_value(radarNode, "RADAR_TYPE");
        
        // Test
        pugi::xml_node testNode = root.child("test");
        TEST_TYPE = get_node_value(testNode, "TEST_TYPE");
        CONT = get_node_value_as_bool(testNode, "CONT");
        RX_START_TIME = get_node_value_as_double(testNode, "RX_START_TIME");
        TX_START_TIME = get_node_value_as_double(testNode, "TX_START_TIME");
        NUM_SAMPLES = get_node_value_as_double(testNode,"NUM_SAMPLES");
        // Options
        pugi::xml_node optionsNode = root.child("options");
        OUTPUT_FILE = get_node_value(optionsNode, "OUTPUT_FILE");
        std::stringstream ss(optionsNode.child_value("VERBOSE"));
        bool b;
        if(!(ss >> std::boolalpha >> b)) {
            std::cerr << "Verbose bool error" << std::endl;
            return -1;
        }
        VERBOSE = b;


        return 0;
    }




    bool usrp_config::setUSRP_mode_from_config()
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

    int usrp_config::connect(uhd::usrp::multi_usrp::sptr& usrp)
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
        try
        {
            usrp = uhd::usrp::multi_usrp::make(std::string("addr=") += SDR_IP);
        }
        catch(const std::exception& e)
        {
            std::cerr << e.what() << '\n';
            return -1;
        }
        
        // 4. Get possible parameters
        // 5. Check specified config is within possible parameters
        if (checkPossibleParams(usrp) != 0)
        {
            return -1;
        }


        
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
            setupTransmitter(usrp);
            break;
        case config::USRP_MODE::RX_ONLY_MODE:
            setupReceiever(usrp);
            break;
        case config::USRP_MODE::TX_AND_RX_MODE:
            setupTransmitter(usrp);
            setupReceiever(usrp);
            break;
        default:
            std::cout << "Hit default case, mode not set correctly" << std::endl;
            break;
        }
        return 0;
    }


    int usrp_config::checkPossibleParams(uhd::usrp::multi_usrp::sptr& usrp)
    {

        bool found = false;
        int err = 0;
        // -------------------------------------------------------------------
        pugi::xml_node configNode = root.child("device");
        pugi::xml_node freqNode = root.child("frequency");
        // RX PARAMS
        // subdevice

        // dc offset range
        if (freqNode.child("RX_DC_OFFSET"))
        {
            uhd::meta_range_t rx_dc_offset_range = usrp->get_rx_dc_offset_range();
            if (RX_DC_OFFSET > rx_dc_offset_range.stop() || RX_DC_OFFSET < rx_dc_offset_range.start())
            {
                std::cerr << "RX DC offset is outside of tunable range: " << RX_DC_OFFSET << std::endl;
                std::cerr << "Tunable range is from: " << rx_dc_offset_range.start() << " to " << rx_dc_offset_range.stop() << std::endl;
                err = -1;
            }
        }

        // filters
        if (configNode.child("RX_FILTER"))
        {
            std::vector<std::string> rx_filters = usrp->get_rx_filter_names(0);
            utils::ppVector(rx_filters);
        }


        // antenna
        // TX/RX - can be set to either tx or rx
        // RX2 - set to receive
        // CAL - ?
        if (configNode.child("RX_ANTENNA"))
        {
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
        }


        // sample rate?
        if (freqNode.child("RX_RATE"))
        {
            uhd::meta_range_t rx_rates = usrp->get_rx_rates(0);
            if (RX_RATE > rx_rates.stop() || RX_RATE < rx_rates.start())
            {
                std::cerr << "RX rate is outside of tunable range: " << RX_RATE << std::endl;
                std::cerr << "Tunable range is from: " << rx_rates.start() << " to " << rx_rates.stop() << std::endl;
                err = -1;
            }
        }


        // bandwidth range
        if (freqNode.child("RX_BW"))
        {
            uhd::meta_range_t rx_bandwidth_range = usrp->get_rx_bandwidth_range(0);
            if (RX_BW > rx_bandwidth_range.stop())
            {
                std::cerr << "Specified RX bandwidth is too large: " << RX_BW << std::endl;
                std::cerr << "Maximum RX bandwidth: " << rx_bandwidth_range.stop() << std::endl;
                err = -1;
            }
        }


        // freq range
        if (freqNode.child("RX_FREQ"))
        {
            uhd::freq_range_t rx_freq_range = usrp->get_rx_freq_range(0);
            //std::cout << rx_freq_range.to_pp_string() << std::endl;
            if (RX_FREQ > rx_freq_range.stop() || RX_FREQ < rx_freq_range.start())
            {
                std::cerr << "RX centre freq is outside of tunable range: " << RX_FREQ << std::endl;
                std::cerr << "Tunable range is from: " << rx_freq_range.start() << " to " << rx_freq_range.stop() << std::endl;
                err = -1;
            }
        }


        
        // gain names
        if (freqNode.child("RX_GAIN_NAME"))
        {
            std::vector<std::string> rx_gain_names = usrp->get_rx_gain_names(0);
            utils::ppVector(rx_gain_names);
        }


        // gain profile names
        if (freqNode.child("RX_GAIN_PROFILE"))
        {
            std::vector<std::string> rx_gain_profile_names = usrp->get_rx_gain_profile_names(0);
            utils::ppVector(rx_gain_profile_names);
        }


        // gain range
        if (freqNode.child("RX_GAIN"))
        {
            uhd::gain_range_t rx_gain_range = usrp->get_rx_gain_range(0);
            if (RX_GAIN > rx_gain_range.stop() || RX_GAIN < rx_gain_range.start())
            {
                std::cerr << "RX gain is outside of tunable range: " << RX_GAIN << std::endl;
                std::cerr << "Tunable range is from: " << rx_gain_range.start() << " to " << rx_gain_range.stop() << std::endl;
                err = -1;            
            }
        }

        // power range
        try
        {
            uhd::meta_range_t rx_power_range = usrp->get_rx_power_range(0);
            std::cout << "RX power range:" << std::endl;
            std::cout << rx_power_range.start() << std::endl;
            std::cout << rx_power_range.stop() << std::endl;            
        }
        catch(const std::exception& e)
        {
            std::cerr << e.what() << '\n';
        }
        


        // LO freq range
        // LO names
        // LO sources
        // num channels

        // sensors

        // -------------------------------------------------------------------
        // TX PARAMS
        // subdevice

        // dc offset range
        if (freqNode.child("TX_DC_OFFSET"))
        {
            uhd::meta_range_t tx_dc_offset_range = usrp->get_tx_dc_offset_range();
            if (TX_DC_OFFSET > tx_dc_offset_range.stop() || TX_DC_OFFSET < tx_dc_offset_range.start())
            {
                std::cerr << "TX DC offset is outside of tunable range: " << TX_DC_OFFSET << std::endl;
                std::cerr << "Tunable range is from: " << tx_dc_offset_range.start() << " to " << tx_dc_offset_range.stop() << std::endl;
                err = -1;
            }
        }

        // filters
        if (configNode.child("TX_FILTER"))
        {
            std::vector<std::string> tx_filters = usrp->get_tx_filter_names(0);
            utils::ppVector(tx_filters);
        }

        // antenna
        // TX/RX - can be set to either tx or rx
        // CAL - ?
        if (configNode.child("TX_ANTENNA"))
        {
            std::vector<std::string> tx_antennas = usrp->get_tx_antennas(0);
            found = false;
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
        }


        // sample rate?
        if (freqNode.child("TX_RATE"))
        {
            uhd::meta_range_t tx_rates = usrp->get_tx_rates(0);
            if (TX_RATE > tx_rates.stop() || TX_RATE < tx_rates.start())
            {
                std::cerr << "TX rate is outside of tunable range: " << TX_RATE << std::endl;
                std::cerr << "Tunable range is from: " << tx_rates.start() << " to " << tx_rates.stop() << std::endl;
                err = -1;
            } 
        }


        // bandwidth range
        if (freqNode.child("TX_BW"))
        {
            uhd::meta_range_t tx_bandwidth_range = usrp->get_tx_bandwidth_range(0);
            if (TX_BW > tx_bandwidth_range.stop())
            {
                std::cerr << "Specified TX bandwidth is too large: " << TX_BW << std::endl;
                std::cerr << "Maximum TX bandwidth: " << tx_bandwidth_range.stop() << std::endl;
                err = -1;
            }
        }


        // freq range
        if (freqNode.child("TX_FREQ"))
        {
            uhd::freq_range_t tx_freq_range = usrp->get_tx_freq_range(0);
            if (TX_FREQ > tx_freq_range.stop() || TX_FREQ < tx_freq_range.start())
            {
                std::cerr << "TX centre freq is outside of tunable range: " << TX_FREQ << std::endl;
                std::cerr << "Tunable range is from: " << tx_freq_range.start() << " to " << tx_freq_range.stop() << std::endl;
                err = -1;
            }
        }


        
        // gain names
        if (freqNode.child("TX_GAIN_NAME"))
        {
            std::vector<std::string> tx_gain_names = usrp->get_tx_gain_names(0);
            utils::ppVector(tx_gain_names);
        }


        // gain profile names
        if (freqNode.child("TX_GAIN_PROFILE"))
        {
            std::vector<std::string> tx_gain_profile_names = usrp->get_tx_gain_profile_names(0);
            utils::ppVector(tx_gain_profile_names);
        }


        // gain range
        if (freqNode.child("TX_GAIN"))
        {
            uhd::gain_range_t tx_gain_range = usrp->get_tx_gain_range(0);
            if (TX_GAIN > tx_gain_range.stop() || TX_GAIN < tx_gain_range.start())
            {
                std::cerr << "TX gain is outside of tunable range: " << TX_GAIN << std::endl;
                std::cerr << "Tunable range is from: " << tx_gain_range.start() << " to " << tx_gain_range.stop() << std::endl;
                err = -1;            
            }
        }

        // power range
        try
        {
            uhd::meta_range_t tx_power_range = usrp->get_tx_power_range(0);
            std::cout << "TX power range:" << std::endl;
            std::cout << tx_power_range.start() << std::endl;
            std::cout << tx_power_range.stop() << std::endl;            
        }
        catch(const std::exception& e)
        {
            std::cerr << e.what() << '\n';
        }


        // LO freq range
        // LO names
        // LO sources
        // num channels

        // sensors


        // Global?
        // time sources
        // sync sources
        return err;
    }


    // SETUP RECEIVER ---------------------------------------------------------------------------------------------
    int usrp_config::setupReceiever(uhd::usrp::multi_usrp::sptr& rx_usrp){
        // clock should already be set up for whole device
        rx_usrp->set_rx_subdev_spec(RX_SUBDEV);
        rx_usrp->set_rx_antenna(RX_ANTENNA);
        
        pugi::xml_node freqNode = root.child("frequency");

        // sample rate
        if (freqNode.child("RX_RATE"))
        {
            double rx_rate=RX_RATE;
            std::cout << "Setting RX Rate (MHz):  "<< (rx_rate / 1e6)<< std::endl;
            rx_usrp->set_rx_rate(rx_rate);
            double actualRate=rx_usrp->get_rx_rate();
            if (rx_rate=!actualRate){
                std::cout << "Actual RX Rate (MHz) : "<< (actualRate / 1e6)<<". (Overwritten config) n";
                RX_RATE=actualRate;
            }
        }




        // bandwidth
        if (freqNode.child("RX_BW"))
        {
            std::cout << "Setting RX bandwidth (MHz):   " << (RX_BW / 1e6)  << std::endl;
            rx_usrp->set_rx_bandwidth(RX_BW);
            double actualBW = rx_usrp->get_rx_bandwidth();
            if (RX_BW != actualBW){
                std::cout << "Actual RX bandwidth (MHz) : "<< (actualBW / 1e6)<<". (Overwritten config) n";
                RX_BW=actualBW;
            }    
        }



        // center freq
        if (freqNode.child("RX_FREQ"))
        {
            double rx_center_freq= RX_FREQ;
            std::cout << "Setting RX center freq (MHz): " << rx_center_freq / 1e6 << std::endl;
            uhd::tune_request_t rx_tune_req(RX_FREQ); // LO offset optional.. todo?
            uhd::tune_result_t rx_tune_res = rx_usrp->set_rx_freq(rx_tune_req);

            if(std::abs(rx_usrp->get_rx_freq()-RX_FREQ)>100){
                std::cerr<<"setting of center freq unsuccessful. Requested: "<< (double)RX_FREQ/1e6<<" Error: "<<rx_usrp->get_rx_freq()-RX_FREQ<<"\n";    
            }
        }




        // gain
        if (freqNode.child("RX_GAIN"))
        {
            double rx_gain = RX_GAIN;
            std::cout << "Setting RX Gain (dB) : " << rx_gain<< std::endl;
            rx_usrp->set_rx_gain(rx_gain);
            std::cout << "Actual RX Gain (dB) : " << rx_usrp->get_rx_gain() <<". Out of a possible range:"<< rx_usrp->get_rx_gain_range().to_pp_string() << std::endl;
        }


        // make sure LO locked (give it a few attempts)
        size_t numlockAttempts=0;
        while(numlockAttempts < 5 && !confirmRxOscillatorsLocked(rx_usrp, REF_CLOCK, true)){
            confirmRxOscillatorsLocked(rx_usrp,REF_CLOCK,true);
            sleep(1.0); // parametrize
            numlockAttempts++;
            
        }
        

        // Reporting if sources locked in time
        std::vector<std::string> rx_sensor_names = rx_usrp->get_rx_sensor_names(0);
        uhd::sensor_value_t lo_locked = rx_usrp->get_rx_sensor("lo_locked", 0);
        if (std::find(rx_sensor_names.begin(), rx_sensor_names.end(), "lo_locked")!= rx_sensor_names.end()){
            
            if(!lo_locked.to_bool()){
                std::cout << "LO failed to lock in time." << std::endl;
            }
            else
            {
                std::cout << "LO locked in time." << std::endl;
            }
        }
        // 
        rx_sensor_names = rx_usrp->get_mboard_sensor_names(0);
        if ((REF_CLOCK == "mimo") and (std::find(rx_sensor_names.begin(), rx_sensor_names.end(), "mimo_locked")!= rx_sensor_names.end())) {
            uhd::sensor_value_t lo_locked = rx_usrp->get_rx_sensor("mimo_locked", 0);
            if(!lo_locked.to_bool()){
                std::cout << "MIMO failed to lock in time." << std::endl;
            }
            else
            {
                std::cout << "MIMO locked in time." << std::endl;
            }
        }
        //
        if ((REF_CLOCK == "external") and (std::find(rx_sensor_names.begin(), rx_sensor_names.end(), "ref_locked")!= rx_sensor_names.end())) {
            uhd::sensor_value_t lo_locked = rx_usrp->get_rx_sensor("lo_locked", 0);
            if(!lo_locked.to_bool()){
                std::cout << "External clock failed to lock in time." << std::endl;
            }
            else
            {
                std::cout << "External clock locked in time." << std::endl;
            }
        }
        std::cout << std::endl;



        //required for non-modulated waveforms. (Rory, you should probably remove this)
        rx_usrp->set_rx_dc_offset(false);

        
        if (rx_usrp->get_num_mboards() > 1) {
            rx_usrp->set_time_unknown_pps(uhd::time_spec_t(0.0));
        }
        
        // 
        return 0;
        
    } //setupReceiver


    /**
     * Confirm LO, MIMO and REF clock sources locked
     * 
     */
    bool usrp_config::confirmRxOscillatorsLocked(uhd::usrp::multi_usrp::sptr& usrp_object, std::string ref_source,bool printing){
        std::string clock_source = ref_source;
        std::vector<std::string> rx_sensor_names;
        //checking LO
        rx_sensor_names = usrp_object->get_rx_sensor_names();
        if (std::find(rx_sensor_names.begin(), rx_sensor_names.end(), "lo_locked")!= rx_sensor_names.end()){
            uhd::sensor_value_t lo_locked = usrp_object->get_rx_sensor("lo_locked");
            if(printing){std::cout << boost::format("Checking RX.... %s ...") % lo_locked.to_pp_string()  << std::endl;}
            if(!lo_locked.to_bool()){
                return false;
            }
        }
        //checking for mimo 
        rx_sensor_names = usrp_object->get_mboard_sensor_names();
        if ((clock_source == "mimo") and (std::find(rx_sensor_names.begin(), rx_sensor_names.end(), "mimo_locked")!= rx_sensor_names.end())) {
            uhd::sensor_value_t mimo_locked = usrp_object->get_mboard_sensor("mimo_locked", 0);
            if(printing){std::cout << boost::format("Checking RX .... %s ...") % mimo_locked.to_pp_string()<< std::endl;}
            if(!mimo_locked.to_bool()){
                return false;
            }
        }
        // checking external
        if ((clock_source == "external") and (std::find(rx_sensor_names.begin(), rx_sensor_names.end(), "ref_locked")!= rx_sensor_names.end())) {
            uhd::sensor_value_t ref_locked = usrp_object->get_mboard_sensor("ref_locked", 0);
            if(printing){std::cout << boost::format("Checking RX .... : %s ...") % ref_locked.to_pp_string() << std::endl;}
            if(!ref_locked.to_bool()){
                return false;
            }
        }
        return true;
    }

    /** 
    bool usrp_config::incrementRxFreq(uhd::usrp::multi_usrp::sptr rx_usrp, double incrementFreqHz){
        double newFreq = rx_usrp->get_rx_freq()+incrementFreqHz;
        if(newFreq<MIN_FREQ){
            std::cerr<<"Requested Frequency Lower than SBX board capable of. If not using SBX, edit config_constants.hpp\n";
            return false;
        }
        if(newFreq>MAX_FREQ){
            std::cerr<<"Requested Frequency higher than SBX board capable of. If not using SBX, edit config_constants.hpp\n";
            return false;
        }

        return setRXFreqHz(rx_usrp,newFreq);
    }// incrementRxFreq()
    **/


   // TX CONFIG -------------------------------------------------------------------------------------------------
    bool usrp_config::confirmTxOscillatorsLocked(uhd::usrp::multi_usrp::sptr& usrp_object, std::string ref_source, bool printing){
        std::string clock_source = ref_source;
        std::vector<std::string> tx_sensor_names;
        tx_sensor_names = usrp_object->get_tx_sensor_names(0);

        // checking LO
        if (std::find(tx_sensor_names.begin(), tx_sensor_names.end(), "lo_locked")!= tx_sensor_names.end()){
            uhd::sensor_value_t lo_locked = usrp_object->get_tx_sensor("lo_locked", 0);
            if(printing){std::cout << boost::format("Checking TX.... %s ...") % lo_locked.to_pp_string()  << std::endl;}
            if(!lo_locked.to_bool()){
                return false;
            }
        }
        //
        tx_sensor_names = usrp_object->get_mboard_sensor_names(0);
        if ((clock_source == "mimo") and (std::find(tx_sensor_names.begin(), tx_sensor_names.end(), "mimo_locked")!= tx_sensor_names.end())) {
            uhd::sensor_value_t mimo_locked = usrp_object->get_mboard_sensor("mimo_locked", 0);
            if(printing){std::cout << boost::format("Checking TX .... %s ...") % mimo_locked.to_pp_string()<< std::endl;}
            if(!mimo_locked.to_bool()){
                return false;
            }
        }
        //
        if ((clock_source == "external") and (std::find(tx_sensor_names.begin(), tx_sensor_names.end(), "ref_locked")!= tx_sensor_names.end())) {
            uhd::sensor_value_t ref_locked = usrp_object->get_mboard_sensor("ref_locked", 0);
            if(printing){std::cout << boost::format("Checking TX .... : %s ...") % ref_locked.to_pp_string() << std::endl;}
            if(!ref_locked.to_bool()){
                return false;
            }
        }
        return true;
    }

    int usrp_config::setupTransmitter(uhd::usrp::multi_usrp::sptr& tx_usrp){
        //ref clock already set up

        
        tx_usrp->set_tx_subdev_spec(TX_SUBDEV);
        tx_usrp->set_tx_antenna(TX_ANTENNA);

        pugi::xml_node freqNode = root.child("frequency");

        // sample rate
        if (freqNode.child("TX_RATE"))
        {
            double tx_rate=TX_RATE;
            std::cout << "Setting TX Rate (MHz):  "<< (tx_rate / 1e6)<< std::endl;
            tx_usrp->set_tx_rate(tx_rate);
            double actualRate=tx_usrp->get_tx_rate();
            if (tx_rate=!actualRate){
                std::cout << "Actual TX Rate (MHz) : "<< (actualRate / 1e6)<<". (Overwritten config) n";
                TX_RATE=actualRate;
            }
        }

        
        
        // bandwidth
        if (freqNode.child("TX_BW"))
        {
            std::cout << "Setting TX bandwidth (MHz):  "<< (TX_BW / 1e6)<< std::endl;
            tx_usrp->set_tx_bandwidth(TX_BW);
            double actualBW = tx_usrp->get_tx_bandwidth();
            if (TX_BW != actualBW){
                std::cout << "Actual TX Bandwidth (MHz) : "<< (actualBW / 1e6)<<". (Overwritten config) n";
                TX_RATE=actualBW;
            }
        }



        // center freq
        if (freqNode.child("TX_FREQ"))
        {
            double tx_center_freq= TX_FREQ;
            std::cout << "Setting TX center freq (MHz):  " << (TX_FREQ / 1e6) << std::endl;
            uhd::tune_request_t tx_tune_req(TX_FREQ);
            tx_usrp->set_tx_freq(tx_tune_req);

            if((std::abs(tx_usrp->get_tx_freq()-TX_FREQ)>100)){ // if more than 100Hz off 
                std::cerr<<"setting of center freq unsuccessful. Requested: "<< (double)TX_FREQ/1e6<<" Error: "<<tx_usrp->get_tx_freq()-TX_FREQ<<"\n";    
            }
        }



        // gain
        if (freqNode.child("TX_GAIN"))
        {
            double tx_gain = TX_GAIN;
            std::cout << "Setting TX Gain (dB) : " << tx_gain<< std::endl;
            tx_usrp->set_tx_gain(tx_gain);
            std::cout << "Actual TX Gain (dB) : " << tx_usrp->get_tx_gain() <<". Out of a possible range:"<< tx_usrp->get_tx_gain_range().to_pp_string() << std::endl;
     
        }

        //tx_usrp->set_rx_dc_offset(false);
        
        // make sure LO locked (give it a few attempts)
        // TODO: parametrize number attempts in xml
        size_t numlockAttempts=0;
        while( numlockAttempts<5&&!confirmTxOscillatorsLocked(tx_usrp,REF_CLOCK,true)){
            numlockAttempts++;
        }
        //
        std::vector<std::string> tx_sensor_names = tx_usrp->get_tx_sensor_names(0);
        if (std::find(tx_sensor_names.begin(), tx_sensor_names.end(), "lo_locked")!= tx_sensor_names.end()){
            uhd::sensor_value_t lo_locked = tx_usrp->get_tx_sensor("lo_locked", 0);
            if(!lo_locked.to_bool()){
                std::cout << "LO failed to lock in time." << std::endl;
            }
            else
            {
                std::cout << "LO locked in time." << std::endl;
            }
        }
        // 
        tx_sensor_names = tx_usrp->get_mboard_sensor_names(0);
        if ((REF_CLOCK == "mimo") and (std::find(tx_sensor_names.begin(), tx_sensor_names.end(), "mimo_locked")!= tx_sensor_names.end())) {
            uhd::sensor_value_t lo_locked = tx_usrp->get_tx_sensor("mimo_locked", 0);
            if(!lo_locked.to_bool()){
                std::cout << "MIMO failed to lock in time." << std::endl;
            }
            else
            {
                std::cout << "MIMO locked in time." << std::endl;
            }
        }
        //
        tx_sensor_names = tx_usrp->get_mboard_sensor_names(0);
        if ((REF_CLOCK == "external") and (std::find(tx_sensor_names.begin(), tx_sensor_names.end(), "ref_locked")!= tx_sensor_names.end())) {
            uhd::sensor_value_t lo_locked = tx_usrp->get_tx_sensor("lo_locked", 0);
            if(!lo_locked.to_bool()){
                std::cout << "External clock failed to lock in time." << std::endl;
            }
            else
            {
                std::cout << "External clock locked in time." << std::endl;
            }
        }
        std::cout << std::endl;
        return 0;


    }



    /**
    bool incrementTxFreqHz(uhd::usrp::multi_usrp::sptr tx_usrp, double freqIncHz){
        double newFreq = tx_usrp->get_tx_freq()+freqIncHz;
        if(newFreq<config::MIN_FREQ){
            std::cerr<<"Requested Frequency Lower than SBX board capable of. If not using SBX, edit config_constants.hpp\n";
            return false;
        }
        if(newFreq>config::MAX_FREQ){
            std::cerr<<"Requested Frequency higher than SBX board capable of. If not using SBX, edit config_constants.hpp\n";
            return false;
        }

        return setTxFreqHz(tx_usrp, newFreq);

    }
    */


    // MISC ------------------------------------------------------------------------------------------------------
    // Function to get all present nodes and their values
    std::vector<std::string> usrp_config::get_present_nodes(const pugi::xml_node& parent_node) {
        std::vector<std::string> nodes_list;
        for (pugi::xml_node node = parent_node.first_child(); node; node = node.next_sibling()) {
            nodes_list.push_back(node.name());
        }
        return nodes_list;
    }


    std::string usrp_config::get_test_type() 
    {
        return TEST_TYPE;
    }
    std::string usrp_config::get_addr()
    {
        return SDR_IP;
    }
    pugi::xml_node usrp_config::get_test_node()
    {
        pugi::xml_node testNode = root.child("test");
        return testNode;
    }
    size_t usrp_config::get_num_samples()
    {
        return NUM_SAMPLES;
    }
    double usrp_config::get_rx_start_time()
    {
        return RX_START_TIME;
    }
    double usrp_config::get_tx_start_time()
    {
        return TX_START_TIME;
    }
    std::string usrp_config::get_waveform_filename()
    {
        return WAVEFORM_FILE;
    }
    double usrp_config::get_tx_rate()
    {
        return TX_RATE;
    }
    bool usrp_config::get_cont()
    {
        return CONT;
    }
}

