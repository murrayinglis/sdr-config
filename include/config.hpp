#ifndef CONFIG_HPP
#define CONFIG_HPP

#include "pugixml.hpp" 
#include <uhd/usrp/multi_usrp.hpp>
#include <string>
#include <iostream>
#include <vector>

namespace config
{
    //TODO: separate these enums
    enum USRP_MODE
    {
        TX_ONLY_MODE=0,
        RX_ONLY_MODE=1,
        TX_AND_RX_MODE=2,
        INVALID_MODE=-1
    };

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

    extern std::string SDR_IP;
    extern std::string TX_SUBDEV;
    extern std::string RX_SUBDEV;
    extern std::string REF_CLOCK;
    extern std::string TX_ANTENNA;
    extern std::string RX_ANTENNA;
    extern std::string TEST_TYPE;
    extern std::string WAVEFORM_FILE;
    //
    extern double TX_FREQ;
    extern double TX_RATE;
    extern double TX_BW;
    extern double TX_GAIN;
    extern double RX_FREQ;
    extern double RX_RATE;
    extern double RX_BW;
    extern double RX_GAIN;
    //
    extern std::string OUTPUT_FILE;
    extern bool VERBOSE;

    extern double MIN_FREQ;
    extern double MAX_FREQ;

    extern uint64_t experimentZeroTime;

    /**
     * Read the config xml file into memory
     */
    int configFromFile(std::string xmlFile);
    /**
     * Load the config file into a config object
     */
    int load();
    int connect();
    bool setUSRP_mode_from_config();
    int checkPossibleParams(uhd::usrp::multi_usrp::sptr usrp);

    extern pugi::xml_document doc;

};

#endif 