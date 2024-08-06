#include <uhd/utils/thread.hpp>
#include <uhd/utils/safe_main.hpp>
#include <uhd/usrp/multi_usrp.hpp>
#include <uhd/exception.hpp>

#include <boost/program_options.hpp>

#include "pugixml.hpp" 
#include "tests.hpp"
#include "config.hpp"
#include <iostream>
#include <string>



namespace po = boost::program_options;
namespace cli
{
    // Declare the variables to hold option values
    //std::string option_find;
    std::string option_dump;
    std::string option_config;
    std::string option_test;
}

void print_help(const po::options_description &desc) 
{
    std::cout << desc << std::endl;
}

int main(int argc, char *argv[]) 
    {


    // Define the options
    po::options_description desc("Allowed options");
    desc.add_options()
        ("help,h", "Display help message")
        ("find", "Finds and displays the address of all devices connected")
        ("dump,", po::value<std::string>(&cli::option_dump), "Dump the config of a specified device to an xml file")
        ("config", po::value<std::string>(&cli::option_config), "Configure the device at a specific address based on a config xml file")
        ("test", po::value<std::string>(&cli::option_test), "Perform one of the test cases specified in the config.");

    // Parse the command line arguments
    po::variables_map vm;
    try 
    {
        po::store(po::parse_command_line(argc, argv, desc), vm);
        po::notify(vm); // Notify the variables map to update the variables
    } catch (const po::error &e) 
    {
        std::cerr << "Error: " << e.what() << std::endl;
        print_help(desc);
        return 1;
    }

    // Check if no options were provided or help option
    if (vm.empty() || (vm.size() == 1 && vm.count("help")))
    {
        print_help(desc);
        return 0;
    }

    // Process the options
    if (vm.count("find")) 
    {
        if (vm.count("help")) 
        {
            std::cout << "Finds and displays the address of all devices connected" << std::endl;
        }
        uhd::device_addrs_t devices = uhd::device::find(uhd::device_addr_t());
        for (uhd::device_addr_t addr : devices)
        {
            std::cout << addr.to_string() << std::endl;
        }

    }
    if (vm.count("dump")) {
        std::cout << "TODO: implement dumping config from addr. " << cli::option_dump << std::endl;
    }
    if (vm.count("config")) {
        if (vm.count("help")) 
        {
            std::cout << "Configure the device at a specific address based on a \
            config xml file. The address must be specified as an argument." << std::endl;
        }
        std::cout << "Now configuring from: " << cli::option_config << std::endl;

        
        config::usrp_config usrp_config; // Make usrp_config object
        usrp_config.configFromFile(cli::option_config.c_str());
        usrp_config.connect(); // Connect to USRP and configure
    }
    if (vm.count("test"))
    {
        if (vm.count("help")) 
        {
            std::cout << "Run one of the implemented tests. The test must be specified as an argument." << std::endl;
            tests::listTestTypes();
            return 0;
        }
        if (cli::option_test == "")
        {
            std::cerr << "Path to config file not specified in arguments list" << std::endl;
            return -1;
        }

        config::usrp_config usrp_config; // Make usrp_config object
        if (usrp_config.configFromFile(cli::option_test.c_str()) == 0)
        {
            tests::handleTest(usrp_config);
        }
        
    }

    return 0;
}
