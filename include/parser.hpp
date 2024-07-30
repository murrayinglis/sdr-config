#ifndef PARSER_HPP
#define PARSER_HPP

#include "pugixml.hpp" 
#include <string>
#include <iostream>
#include <vector>

namespace parser
{
    /**
     * Config struct so that config parameters can be viewed
     * 
     */
    struct config
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
    };

    /**
     * Read the config xml file into memory
     */
    int readFile(const std::string& xmlFile);
    /**
     * Load the config file into a config object
     */
    int load(config& config);

    extern pugi::xml_document doc;

};

#endif 