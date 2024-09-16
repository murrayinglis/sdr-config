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
        {"TX_FROM_FILE", TX_FROM_FILE},

        {"LOOPBACK_TEST", LOOPBACK_TEST},
        {"LOOPBACK_FROM_FILE", LOOPBACK_FROM_FILE},
        {"LATENCY", LATENCY},

        {"PULSED", PULSED}
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
                tests::misc::writeLinearSweepToFile(25e6,50000,8.4e6,8.6e6,"sweep.csv");
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
                    tests::RX::rx_test(usrp, usrp_config.get_num_samples());
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
            case TX_FROM_FILE:
                if (usrp_config.connect(usrp) == 0)
                {
                    tests::TX::tx_waveform_from_file(usrp, "double" , usrp_config.get_waveform_filename(), "waveform_test.bin", usrp_config.get_tx_start_time());
                }
                break;            
            case LOOPBACK_TEST:
                if (usrp_config.connect(usrp) == 0)
                {
                    // TODO: parametrize in config
                    double tx_start = usrp_config.get_tx_start_time();
                    double rx_start = usrp_config.get_rx_start_time();
                    size_t num_samples = usrp_config.get_num_samples();
                    std::vector<std::complex<double>> buffers(num_samples,std::complex<float>{0.8, 0.0});

                    // call test
                    tests::LOOPBACK::loopback(usrp, buffers, tx_start, rx_start);
                }
                break; 
            case LOOPBACK_FROM_FILE:
                if (usrp_config.connect(usrp) == 0)
                {
                    double tx_start = usrp_config.get_tx_start_time();
                    double rx_start = usrp_config.get_rx_start_time();
                    std::string waveformFilename = usrp_config.get_waveform_filename();

                    tests::LOOPBACK::loopback_from_file(usrp, usrp_config);
                }
                break;
            case PULSED:
                if (usrp_config.connect(usrp) == 0)
                {
                    tests::pulsed::pulsed(usrp, usrp_config);
                }
                break;
            default:
                break;
        }

        // This was to check if the usrp pointer was being passed in correctly (it was) (when tested)
        //std::cout << usrp->get_rx_bandwidth() << std::endl;
    }
}