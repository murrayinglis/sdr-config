#include "tests.hpp"
#include "config.hpp"
#include "cli.hpp"

namespace tests
{    
    const std::map<std::string, TEST_TYPE> typesMap =
    {
        {"HELLO_WORLD", HELLO_WORLD},
        {"GEN_SWEEP", GEN_SWEEP},
        {"CONFIG_TEST", CONFIG_TEST}
    };

    void listTestTypes()
    {
        std::cout << "Possible test types:" << std::endl;
        for (const auto& pair : typesMap)
        {   
            std::cout << pair.first << std::endl;
        }
    }

    void handleTest(std::string option_test) 
    {
        // TODO: check if test is valid
        // TODO: enum switch
        // Check if the key exists in the map
        auto it = typesMap.find(option_test);
        TEST_TYPE t;

        if (it != typesMap.end()) {
            t = it->second;
        } else {
            // Key not found
            std::cerr << "Invalid test case: \"" << option_test << "\"" << std::endl;
        }
        
        switch (t)
        {
            case GEN_SWEEP:
                std::cout << "Performing test: " << option_test << std::endl;
                tests::misc::writeLinearSweepToFile(1000000,3000,1,20000,"sweep.csv");
                break;
            case HELLO_WORLD:
                std::cout << "Performing test: " << option_test << std::endl;
                tests::misc::hello_world(std::string("addr=") + config::SDR_IP);
                break;
            case CONFIG_TEST:
                std::cout << "Performing test: " << option_test << std::endl;
                tests::misc::config_test(cli::option_config);
                break;
            default:
                break;
        }
    }
}