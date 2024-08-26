#include "tests.hpp"
#include "utils.hpp"
#include "hardware.hpp"
#include "config.hpp"

#include <uhd/usrp/multi_usrp.hpp>
#include <uhd/transport/zero_copy.hpp>

#include <iostream>



namespace tests{
    namespace RX {
        int rx_test(uhd::usrp::multi_usrp::sptr rx_usrp, size_t num_samples)
        {
            std::string cpu_format="fc64";
            std::string wire_format="sc16";
            std::vector<size_t> rx_channel_nums(0); //SBX will be set up to only have 1 receive channel
            std::mutex recv_mutex;
            const std::lock_guard<std::mutex> lock(recv_mutex);
            double bw = 10.0;

            /*
            hardware::recv_samples_to_file(rx_usrp,
            cpu_format,
            wire_format,
            rx_channel_nums,
            1,
            "rx_test.bin",
            128,
            12800,
            bw,
            &recv_mutex,
            0.0,
            false,
            false,
            false,
            false,
            "");
            */
            
            captureSingleFreqToFile(rx_usrp, "double", num_samples, "outputs/rx_test", 10.0); // <--------------
            return 0;
        }

        int captureSingleFreqToFile(uhd::usrp::multi_usrp::sptr rx_usrp, std::string precision, size_t numSamples, std::string outputFile, double settling_time){

            //check if file exists
            //utils::fileAlreadyExists(&outputFile,"bin");

            //delete existing file


            // the need for separate recv functions can (and should) be solved with a templated receive function

            // recv to file
            if (precision == "double"){
            uhd::stream_args_t stream_args("fc64","sc16");
            std::vector<size_t> rx_channel_nums(0); 
            stream_args.channels             = rx_channel_nums;
            uhd::rx_streamer::sptr rx_stream = rx_usrp->get_rx_stream(stream_args);
                hardware::recv_to_file_doubles(rx_usrp,rx_stream,outputFile,numSamples,settling_time,true); //TODO: set up metadata dept config parameter
                return 0;
                //
            }/*else if (type == "float"){
                UTIL::recv_to_file<std::complex<float>>(rx_usrp, "fc32", "sc16", CONFIG::OUTPUT_FILE, 0, total_num_samps, 1, 0,pstop_signal);
            }else if (type == "short"){
                UTIL::recv_to_file<std::complex<short>>(rx_usrp, "sc16", "sc16", CONFIG::OUTPUT_FILE, 0, total_num_samps, 1, 0,pstop_signal);
            }*/else {
                // clean up transmit worker
                throw std::runtime_error("Unknown type " + precision);
            }
            return 0;
        }


    }

} // namespace test_types