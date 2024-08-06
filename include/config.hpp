#ifndef CONFIG_HPP
#define CONFIG_HPP

#include "pugixml.hpp" 
#include <uhd/usrp/multi_usrp.hpp>
#include <string>
#include <iostream>
#include <vector>


namespace config
{
    class usrp_config
    {
    public:
        usrp_config();

        // populates a usrp_config object from an xml file
        int configFromFile(std::string xmlFile); 

        // TODO: ambiguous naming? connect could be for streaming maybe
        int connect(); 

        // Getters
        std::string get_test_type();
        std::string get_addr();

    private:
        // device
        std::string SDR_IP;
        // config
        std::string TX_SUBDEV;
        std::string RX_SUBDEV;
        std::string REF_CLOCK;
        std::string TX_ANTENNA;
        std::string RX_ANTENNA;
        std::string TEST_TYPE;
        // antenna
        double TX_FREQ;
        double TX_RATE;
        double TX_BW;
        double TX_GAIN;
        double RX_FREQ;
        double RX_RATE;
        double RX_BW;
        double RX_GAIN;
        // options
        std::string OUTPUT_FILE;
        bool VERBOSE;
        // radar
        std::string RADAR_TYPE;
        std::string WAVEFORM_FILE;

        // xml
        pugi::xml_node root;

        int load();
        bool setUSRP_mode_from_config();
        int checkPossibleParams(uhd::usrp::multi_usrp::sptr usrp);

        int setupReceiever(uhd::usrp::multi_usrp::sptr rx_usrp);
        bool confirmRxOscillatorsLocked(uhd::usrp::multi_usrp::sptr usrp_object, std::string ref_source,bool printing);
        //bool incrementRxFreq(uhd::usrp::multi_usrp::sptr rx_usrp, double incrementFreqHz);

        int setupTransmitter(uhd::usrp::multi_usrp::sptr tx_usrp);
        bool confirmTxOscillatorsLocked(uhd::usrp::multi_usrp::sptr usrp_object, std::string ref_source,bool printing);

        std::vector<std::string> get_present_nodes(const pugi::xml_node& parent_node);
    };

    //TODO: separate these enums
    enum USRP_MODE
    {
        TX_ONLY_MODE=0,
        RX_ONLY_MODE=1,
        TX_AND_RX_MODE=2,
        INVALID_MODE=-1
    };

    // TODO: move these
    enum TEST_TYPES
    {
        INVALID=-1,
        TRANSMIT_SINGLE_FREQ=0,
        RECEIVE_SINGLE_FREQ=1,
        LOOPBACK=2,
        LOOPBACK_SINGLE_DOWNMIX=3,
        LOOPBACK_MULTI_STEPS=4,
        SFCW=5,
        LOOPBACK_MULTI_STEPS_OFFSET=6,
    };


    extern uint64_t experimentZeroTime;
    extern pugi::xml_document doc;
};

#endif 