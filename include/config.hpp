#ifndef CONFIG_HPP
#define CONFIG_HPP

#include "pugixml.hpp" 
#include <string>
#include <iostream>
#include <vector>

namespace config
{
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

    /**
     * Read the config xml file into memory
     */
    int readFile(const std::string& xmlFile);
    /**
     * Load the config file into a config object
     */
    int load();

    extern pugi::xml_document doc;

};

#endif 