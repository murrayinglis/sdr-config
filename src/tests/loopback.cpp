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
            // wait until read is true
            std::unique_lock<std::mutex> lk(mtx);
            cv.wait(lk, [&]{ return ready; });

            while (running) 
            {
                // Check if the stop signal is active
                if (!running) {
                    break;
                }

                // start transmitting
                hardware::transmitDoublesAtTime(tx_usrp, buffers, secondsInFuture, tx_stream, md);
            }


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
            // start transmit and receive (try synchronous starting, will probably be off by 1 clock cycle - still significant)
            hardware::recv_to_file_doubles(usrp, "outputs/loopback_test", buffers.size(), settlingTime, true); // TODO: parametrize in config

            //std::this_thread::sleep_for(std::chrono::seconds(2));
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
            std::thread transmit_thread(transmit_worker,usrp, buffers, secondsInFuture, tx_stream, md);


            // call ready to tx thread
            {
                std::lock_guard<std::mutex> lock(mtx);
                ready = true;
            }
            cv.notify_one();
            // start transmit and receive (try synchronous starting, will probably be off by 1 clock cycle - still significant)
            hardware::recv_to_file_doubles(usrp, "outputs/loopback_from_file_test", buffers.size(), settlingTime, true); // TODO: parametrize in config

            //std::this_thread::sleep_for(std::chrono::seconds(2));
            running = false;

            // wait for transmit thread to finish
            transmit_thread.join();
        }

        void latency(uhd::usrp::multi_usrp::sptr usrp, std::vector<std::complex<double>> buffers, double secondsInFuture, double settlingTime)
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
            // start transmit and receive (try synchronous starting, will probably be off by 1 clock cycle - still significant)
            hardware::recv_to_file_doubles(usrp, "outputs/latency_test", buffers.size(), settlingTime, true); // TODO: filename/file directory in xml

            running = false;

            transmit_thread.join();            

        }
    }
}