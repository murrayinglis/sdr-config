#include "tests.hpp"
#include "config.hpp"

namespace tests
{
    void handleTest(std::string option_test) 
    {
        // TODO: check if test is valid
        // TODO: enum switch
        
        if (option_test == "gen_sweep") 
        {
            std::cout << "Performing test: " << option_test << std::endl;
            tests::misc::writeLinearSweepToFile(1000000,3000,1,20000,"sweep.csv");
        }
        if (option_test == "hello_world")
        {
            std::cout << "Performing test: " << option_test << std::endl;
            if (config::configFromFile("configs/config.xml") == 0)
            {
                tests::misc::hello_world(std::string("addr=") + config::SDR_IP);
            }  
            else
            {

            } 
        }
        if (option_test == "config_test")
        {
            std::cout << "Performing test: " << option_test << std::endl;
            tests::misc::config_test(option_test.c_str());
        }
    }
}