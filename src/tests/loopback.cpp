#include "tests.hpp"
#include "hardware.hpp"
#include "utils.hpp"

#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>

#include <boost/thread/thread.hpp>

namespace tests
{
    namespace LOOPBACK
    {
        std::mutex mtx;
        std::condition_variable cv;
        bool ready = false;

        // Shared state for controlling the thread
        std::atomic<bool> running(true);

        void transmit_worker(uhd::usrp::multi_usrp::sptr tx_usrp, 
            std::vector<std::complex<double>> buffers, 
            double secondsInFuture, 
            uhd::tx_streamer::sptr tx_stream, 
            uhd::tx_metadata_t md)
        {
            secondsInFuture += 2e-3;
            hardware::transmitDoublesAtTime(tx_usrp, buffers, secondsInFuture, tx_stream, md);
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

        void loopback(uhd::usrp::multi_usrp::sptr usrp, std::vector<std::complex<double>> buffers, double secondsInFuture, double settlingTime)
        {
            //set up transmit streamer
            uhd::stream_args_t stream_args("fc64","sc16");
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
            std::thread transmit_thread(transmit_worker,usrp, buffers, secondsInFuture, tx_stream, md);


            // call ready to tx thread
            {
                std::lock_guard<std::mutex> lock(mtx);
                ready = true;
            }
            cv.notify_one();
            // start receive (try synchronous starting, will probably be off by 1 clock cycle - still significant)
            
            hardware::recv_to_file_doubles(usrp, rx_stream, "outputs/loopback_test", buffers.size(), settlingTime, true); // TODO: parametrize in config


            // stop transmitting  
            //std::this_thread::sleep_for(std::chrono::seconds(2)); // TODO: parametrize in config  
            hardware::tx_stop_flag.store(true);

            // wait for transmit thread to finish
            transmit_thread.join();
        }

        void loopback_from_file(uhd::usrp::multi_usrp::sptr usrp, config::usrp_config usrp_config)
        {
            // extract params from config
            std::string waveformFilename = usrp_config.get_waveform_filename();
            double secondsInFuture = usrp_config.get_tx_start_time();
            double settlingTime = usrp_config.get_rx_start_time();
            double numRequestedSamples = usrp_config.get_num_samples();

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

            boost::thread_group thread_group;
            thread_group.create_thread(boost::bind(&receive_worker, usrp, "outputs/loopback_from_file_test", usrp_config.get_num_samples(),
            usrp_config.get_rx_start_time(), rx_stream, true));
            thread_group.create_thread(boost::bind(&transmit_worker, usrp, tx_buffers, usrp_config.get_tx_start_time(), tx_stream, md));

            // stop transmitting  
            std::this_thread::sleep_for(std::chrono::microseconds((int)usrp_config.get_tx_start_time()*1000)); // have to wait at LEAST the amount of time we set commands to execute in the future
            hardware::tx_stop_flag.store(true);

            thread_group.join_all();
        }





    }
}