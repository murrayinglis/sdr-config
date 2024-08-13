#include "tests.hpp"
#include "config.hpp"
#include "cli.hpp"


namespace tests
{    
    const std::map<std::string, TEST_TYPE> typesMap =
    {
        {"HELLO_WORLD", HELLO_WORLD},
        {"GEN_SWEEP", GEN_SWEEP},
        {"CONFIG_TEST", CONFIG_TEST},
        {"RX_TEST", RX_TEST},
        {"TX_TEST", TX_TEST},
        {"TX_SINGLE_FREQ", TX_SINGLE_FREQ},
        {"LOOPBACK", LOOPBACK_TEST}
    };


    void listTestTypes()
    {
        std::cout << "Possible test types:" << std::endl;
        for (const auto& pair : typesMap)
        {   
            std::cout << pair.first << std::endl;
        }
    }


    void handleTest(config::usrp_config usrp_config) 
    {
        std::string option_test = usrp_config.get_test_type();

        // Check if the key exists in the map
        auto it = typesMap.find(option_test);
        TEST_TYPE t;

        if (it != typesMap.end()) {
            t = it->second;
        } else {
            // Key not found
            std::cerr << "Invalid test case: \"" << option_test << "\"" << std::endl;
        }

        // usrp object needed for some tests
        uhd::usrp::multi_usrp::sptr usrp; 
        std::cout << "Performing test " << option_test << std::endl;
        switch (t)
        {
            case GEN_SWEEP:
                tests::misc::writeLinearSweepToFile(1000000,3000,1,20000,"sweep.csv");
                break;
            case HELLO_WORLD:
                tests::misc::hello_world(std::string("addr=") + usrp_config.get_addr());
                break;
            case CONFIG_TEST:
                tests::misc::config_test(usrp_config);
                break;
            case RX_TEST:
                if (usrp_config.connect(usrp) == 0)
                {
                    tests::RX::rx_test(usrp);
                }
                break;
            case TX_TEST:
                if (usrp_config.connect(usrp) == 0)
                {
                    tests::TX::tx_test(usrp);
                }
                break;
            case TX_SINGLE_FREQ:
                // configFromFile() called already
                if (usrp_config.connect(usrp) == 0)
                {
                    tests::TX::transmitSingleFreq(usrp);
                }
                break;              
            case LOOPBACK_TEST:
                if (usrp_config.connect(usrp) == 0)
                {
                    tests::LOOPBACK::loopback(usrp);
                }
                break; 
            default:
                break;
        }
    }
}