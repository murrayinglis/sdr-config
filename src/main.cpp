#include <uhd/utils/thread.hpp>
#include <uhd/utils/safe_main.hpp>
#include <uhd/usrp/multi_usrp.hpp>
#include <uhd/exception.hpp>

#include <boost/program_options.hpp>

#include "pugixml.hpp" 
#include "config.hpp"
#include <iostream>
#include <string>

#include "tests.hpp"

namespace po = boost::program_options;

void print_help(const po::options_description &desc) 
{
    std::cout << desc << std::endl;
}

int main(int argc, char *argv[]) 
    {
    // Declare the variables to hold option values
    //std::string option_find;
    std::string option_dump;
    std::string option_config;
    std::string option_test;

    // Define the options
    po::options_description desc("Allowed options");
    desc.add_options()
        ("help,h", "Display help message")
        ("find", "Finds and displays the address of all devices connected")
        ("dump,", po::value<std::string>(&option_dump), "Dump the config of a specified device to an xml file")
        ("config", po::value<std::string>(&option_config), "Configure the device at a specific address based on a config xml file")
        ("test", po::value<std::string>(&option_test), "Perform one of the test cases defined.");

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
        uhd::device_addrs_t devices = uhd::device::find(uhd::device_addr_t());
        for (uhd::device_addr_t addr : devices)
        {
            std::cout << addr.to_string() << std::endl;
        }

    }
    if (vm.count("dump")) {
        std::cout << "TODO: implement dumping config from addr. " << option_dump << std::endl;
    }
    if (vm.count("config")) {
        std::cout << "Now configuring from: " << option_config << std::endl;
        config::configFromFile(option_config.c_str());
    }
    if (vm.count("test"))
    {
        // TODO: check if test is valid
        // TODO: enum switch
        
        if (option_test == "gen_sweep") 
        {
            std::cout << "Performing test: " << option_test << std::endl;
            tests::misc::writeLinearSweepToFile(1000000,3000,1,20000,"sweep.csv");
        }
    }

    return 0;
}
