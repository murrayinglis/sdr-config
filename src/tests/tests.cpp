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
        {"TX_SINGLE_FREQ", TX_SINGLE_FREQ}
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

        switch (t)
        {
            case GEN_SWEEP:
                std::cout << "Performing test: " << option_test << std::endl;
                tests::misc::writeLinearSweepToFile(1000000,3000,1,20000,"sweep.csv");
                break;
            case HELLO_WORLD:
                std::cout << "Performing test: " << option_test << std::endl;
                tests::misc::hello_world(std::string("addr=") + usrp_config.get_addr());
                break;
            case CONFIG_TEST:
                std::cout << "Performing test: " << option_test << std::endl;
                tests::misc::config_test(usrp_config);
                break;
            case RX_TEST:
                std::cout << "Performing test: " << option_test << std::endl;
                if (usrp_config.connect(usrp) == 0)
                {
                    tests::RX::rx_test(usrp);
                }
                break;
            case TX_TEST:
                std::cout << "Performing test: " << option_test << std::endl;
                if (usrp_config.connect(usrp) == 0)
                {
                    tests::TX::tx_test(usrp);
                }
                break;
            case TX_SINGLE_FREQ:
                // configFromFile() called already
                std::cout << "Performing test: " << option_test << std::endl;
                if (usrp_config.connect(usrp) == 0)
                {
                    tests::TX::transmitSingleFreq(usrp);
                }
                
                break;                
            default:
                break;
        }
    }
}