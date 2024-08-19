#ifndef CONFIG_HPP
#define CONFIG_HPP

#include "pugixml.hpp" 
#include <uhd/usrp/multi_usrp.hpp>
#include <string>
#include <iostream>
#include <vector>

/**
 * @brief Related to the configuration of the USRP device
 * 
 */
namespace config
{
    /**
     * @class usrp_config
     * @brief A USRP config object
     * @details Stores the config from an xml file in an object
     */
    class usrp_config
    {
    public:
        usrp_config();

        /**
         * @brief Populates a usrp_config object from an xml file.
         * 
         * 
         * @param xmlFile The path to the xml file as an `std::string`.
         * @return Result.
         * @retval 0 if succesful.
         * @retval -1 if unsuccesful.
        */ 
        int configFromFile(std::string xmlFile); 

        /**
         * @todo ambiguous naming? connect could be for streaming maybe
         * @brief Connect and configure the device
         * @details Connects to the device at the specified address in the configuration and configures it according
         * to the stored parameters.
         * 
         * @param usrp The USRP device that will be configured
         * 
         * @return Result.
         * @retval 0 if succesful.
         * @retval -1 if unsuccesful.
        */ 
        int connect(uhd::usrp::multi_usrp::sptr& usrp); 

        // Getters
        /**
         * @brief Returns the test type specified in the config
         * @return The test type as an `std::string`
         * 
         */
        std::string get_test_type();

        /**
         * @brief Returns the device address specified in the config
         * @return The device address as an `std::string`
         */
        std::string get_addr();

        /**
         * @brief Returns the test xml node for extracting parameters to use in the test cases.
         * @return The test node as a `pugi::xml_node`
         */
        pugi::xml_node get_test_node();

        /**
         * @brief Returns the number of desired samples
         * @return The number of samples as a `size_t`
         * 
         */
        size_t get_num_samples();

        double get_rx_settling_time();

        double get_tx_start_time();

        std::string get_waveform_filename();


    private:
        // device
        std::string SDR_IP;
        // config
        std::string TX_SUBDEV;
        std::string RX_SUBDEV;
        std::string REF_CLOCK;
        std::string TX_ANTENNA;
        std::string RX_ANTENNA;
        // antenna
        double TX_FREQ;
        double TX_RATE;
        double TX_BW;
        double TX_GAIN;
        double TX_DC_OFFSET;
        double RX_FREQ;
        double RX_RATE;
        double RX_BW;
        double RX_GAIN;
        double RX_DC_OFFSET;
        // test
        std::string TEST_TYPE;
        double RX_SETTLING_TIME;    
        double TX_START_TIME;
        size_t NUM_SAMPLES;
        // radar
        std::string RADAR_TYPE;
        std::string WAVEFORM_FILE;
        // options
        bool VERBOSE;
        std::string OUTPUT_FILE;    

        // xml
        pugi::xml_node root;

        int load();
        bool setUSRP_mode_from_config();
        int checkPossibleParams(uhd::usrp::multi_usrp::sptr& usrp);

        int setupReceiever(uhd::usrp::multi_usrp::sptr& rx_usrp);
        bool confirmRxOscillatorsLocked(uhd::usrp::multi_usrp::sptr& usrp_object, std::string ref_source,bool printing);
        //bool incrementRxFreq(uhd::usrp::multi_usrp::sptr rx_usrp, double incrementFreqHz);

        int setupTransmitter(uhd::usrp::multi_usrp::sptr& tx_usrp);
        bool confirmTxOscillatorsLocked(uhd::usrp::multi_usrp::sptr& usrp_object, std::string ref_source,bool printing);

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