
// This software is modified by Murray Inglis for his final year thesis


#include "tests.hpp"
#include "utils.hpp"
#include "hardware.hpp"
#include <uhd/usrp/multi_usrp.hpp>
#include <uhd/utils/thread.hpp>

#include <iostream>
#include <fstream>
#include <filesystem>
#include <sys/time.h>
#include <sys/resource.h>

#include <boost/thread/thread.hpp>
#include <boost/program_options.hpp>
#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

namespace tests
{
    namespace pulsed
    {

//-----------------------------------------------------------------------------------------------------------------------------
        void transmit_worker(uhd::usrp::multi_usrp::sptr tx_usrp, 
            std::vector<std::complex<double>> buffers, 
            double secondsInFuture, 
            uhd::tx_streamer::sptr tx_stream, 
            uhd::tx_metadata_t md)
        {
            hardware::transmitDoublesUntilStopped(tx_usrp, buffers, secondsInFuture, tx_stream, md);
        }

        void receive_worker(uhd::usrp::multi_usrp::sptr usrp,
            const std::string& file,
            int num_requested_samples,
            double settling_time,
            uhd::rx_streamer::sptr rx_stream,
            bool storeMD)
        {
            hardware::recv_to_file_doubles(usrp, rx_stream, file, num_requested_samples, settling_time, storeMD);
        }

        void pulsed(uhd::usrp::multi_usrp::sptr usrp, config::usrp_config usrp_config)
        {
            // extract params from config
            std::string waveformFilename = usrp_config.get_waveform_filename();
            std::string outputFilename = "outputs/pulsed_test";
            double secondsInFuture = usrp_config.get_tx_start_time();
            double settlingTime = usrp_config.get_rx_start_time();
            double numRequestedSamples = usrp_config.get_num_samples();



            while (true)
            {
                // Read in file 
                // TODO: assuming csv for now
                std::cout << "Reading in: " << waveformFilename << std::endl;


                std::vector<std::complex<double>> tx_buffers = utils::read_in_complex_csv(waveformFilename);

                //set up transmit streamer
                uhd::stream_args_t stream_args("fc64","sc16");
                std::vector<size_t> tx_channel_nums(0);
                stream_args.channels = tx_channel_nums;
                uhd::tx_streamer::sptr tx_stream= usrp->get_tx_stream(stream_args);

                // create a receive streamer - use same cpu and wire format
                std::vector<size_t> rx_channel_nums(0); 
                stream_args.channels             = rx_channel_nums;
                uhd::rx_streamer::sptr rx_stream = usrp->get_rx_stream(stream_args);
                
                uhd::tx_metadata_t md;
                md.has_time_spec=true;
                md.time_spec      =  uhd::time_spec_t(secondsInFuture);
                md.start_of_burst=true;

                // reset usrp time to prepare for transmit/receive
                std::cout << boost::format("Setting device timestamp to 0...") << std::endl;
                usrp->set_time_now(uhd::time_spec_t(0.0));


                // start tx thread with tx worker - this function creates the tx streamer
                // tx thread is waiting for ready call
                std::cout << "TX seconds in future: " << secondsInFuture << std::endl;
                std::cout << "RX settling time: " << settlingTime << std::endl;
                std::cout << "Writing results to: " << outputFilename << std::endl;
                boost::thread_group thread_group;
                thread_group.create_thread(boost::bind(&receive_worker, usrp, outputFilename, usrp_config.get_num_samples(),
                usrp_config.get_rx_start_time(), rx_stream, true));
                thread_group.create_thread(boost::bind(&transmit_worker, usrp, tx_buffers, usrp_config.get_tx_start_time(), tx_stream, md));

                // stop transmitting  
                std::this_thread::sleep_for(std::chrono::milliseconds((int)usrp_config.get_tx_start_time()*1000 + 1000)); // have to wait at LEAST the amount of time we set commands to execute in the future
                //std::this_thread::sleep_for(std::chrono::seconds(4));
                hardware::tx_stop_flag.store(true);

                thread_group.join_all();
                hardware::tx_stop_flag.store(false);
            }

        }
    }
}
