#include "tests.hpp"
#include "hardware.hpp"
#include "utils.hpp"

#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>

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
            // wait until ready is true
            std::unique_lock<std::mutex> lk(mtx);
            cv.wait(lk, [&]{ return ready; });

            hardware::transmitDoublesUntilStopped(tx_usrp, buffers, secondsInFuture, tx_stream, md);

        }

        void transmit_worker_pulse(uhd::usrp::multi_usrp::sptr tx_usrp, 
        std::vector<std::complex<double>> buffers, 
        double secondsInFuture, 
        uhd::tx_streamer::sptr tx_stream, 
        uhd::tx_metadata_t md)
        {
            // wait until ready is true
            std::unique_lock<std::mutex> lk(mtx);
            cv.wait(lk, [&]{ return ready; });
            
            hardware::tx_doublesAtTimeSpec(tx_usrp, buffers, secondsInFuture, tx_stream, md);

        }

        void loopback(uhd::usrp::multi_usrp::sptr usrp, std::vector<std::complex<double>> buffers, double secondsInFuture, double settlingTime)
        {
            //set up transmit streamer
            uhd::stream_args_t stream_args("fc64","sc16");
            uhd::tx_streamer::sptr tx_stream= usrp->get_tx_stream(stream_args);
            
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
            
            hardware::recv_to_file_doubles(usrp, "outputs/loopback_test", buffers.size(), settlingTime, true); // TODO: parametrize in config

            //std::this_thread::sleep_for(std::chrono::seconds(4));

            // stop transmitting
            running = false;

            // wait for transmit thread to finish
            transmit_thread.join();
        }

        void loopback_from_file(uhd::usrp::multi_usrp::sptr usrp, std::string waveformFilename, double secondsInFuture, double settlingTime)
        {
            // Read in file 
            // TODO: assuming csv for now
            std::cout << "Reading in: " << waveformFilename << std::endl;


            std::vector<std::complex<double>> buffers = utils::read_in_complex_csv(waveformFilename);


            //set up transmit streamer
            uhd::stream_args_t stream_args("fc64","sc16");
            uhd::tx_streamer::sptr tx_stream= usrp->get_tx_stream(stream_args);
            
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
            std::thread transmit_thread(transmit_worker,usrp, buffers, secondsInFuture, tx_stream, md);


            
            // call ready to tx thread
            {
                std::lock_guard<std::mutex> lock(mtx);
                ready = true;
            }
            cv.notify_one();
            
            // start receive
            //std::this_thread::sleep_for(std::chrono::seconds(1));
            hardware::recv_to_file_doubles(usrp, "outputs/loopback_from_file_test", 500000, settlingTime, true); // TODO: parametrize in config

            // transmit will have started after receive (assumed?)
            // reverse order here by making receive worker I guess 

            // stop transmitting  
            //std::this_thread::sleep_for(std::chrono::seconds(5));  
            hardware::tx_stop_flag.store(true);

            // wait for transmit thread to finish
            transmit_thread.join();
        }





        /**
         * I think this is a stupid test but I am investigating timing.
         * 
         */
        void latency(uhd::usrp::multi_usrp::sptr usrp, std::vector<std::complex<double>> buffers, double secondsInFuture, double settlingTime)
        {
            // here I am trying to transmit a single pulse and see it in the received data
            // overriding tx and rx start time to be the same
            secondsInFuture = 0.1;
            settlingTime = 0.1;

            //set up transmit streamer
            uhd::stream_args_t stream_args("fc64","sc16");
            uhd::tx_streamer::sptr tx_stream= usrp->get_tx_stream(stream_args);
            uhd::tx_metadata_t md;
            md.has_time_spec=true;
            md.time_spec      =  uhd::time_spec_t(secondsInFuture);
            md.start_of_burst=true;

            //set up receive streamer
            std::vector<size_t> rx_channel_nums(0); //SBX will be set up to only have 1 receive channel
            stream_args.channels             = rx_channel_nums;
            uhd::rx_streamer::sptr rx_stream = usrp->get_rx_stream(stream_args);

            // reset usrp time to prepare for transmit/receive
            std::cout << boost::format("Setting device timestamp to 0...") << std::endl;
            usrp->set_time_now(uhd::time_spec_t(0.0));


            // start tx thread with tx worker - this function creates the tx streamer
            // tx thread is waiting for ready call
            std::thread transmit_thread(transmit_worker_pulse,usrp, buffers, secondsInFuture, tx_stream, md);


            // call ready to tx thread
            {
                std::lock_guard<std::mutex> lock(mtx);
                ready = true;
            }
            cv.notify_one();


            // start receive
            std::vector<std::complex<double>> rx_buffers;
            hardware::rx_doublesAtTimeAlert(usrp, 500000, secondsInFuture, rx_stream);


            transmit_thread.join();            
            std::cout << std::endl;
        }
    }
}