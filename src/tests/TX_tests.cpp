#include <chrono>
#include "tests.hpp"
#include "config.hpp"
#include "utils.hpp"
#include <uhd/usrp/multi_usrp.hpp>
#include "hardware.hpp"

namespace tests{
    namespace TX {
        int tx_test(uhd::usrp::multi_usrp::sptr tx_usrp)
        {
            std::cout << "TX_TEST running" << std::endl;
            std::vector<std::complex<double>> buffers(10000,std::complex<float>{0.8, 0.0});
            double secondsInFuture = 0.5; // <--------------


            //set up transmit streamer
            uhd::stream_args_t stream_args("fc64","sc16");
            uhd::tx_streamer::sptr tx_stream= tx_usrp->get_tx_stream(stream_args);
            
            uhd::tx_metadata_t md;
            md.has_time_spec=true;
            md.time_spec      =  uhd::time_spec_t(secondsInFuture);
            md.start_of_burst=true;

            // reset usrp time to prepare for transmit/receive
            std::cout << boost::format("Setting device timestamp to 0...") << std::endl;
            tx_usrp->set_time_now(uhd::time_spec_t(0.0)); // TODO: use config

            hardware::transmitDoublesAtTime(tx_usrp, buffers, secondsInFuture, tx_stream, md);
            return 0;
        }

        int transmitSingleFreq(uhd::usrp::multi_usrp::sptr tx_usrp){
            //set up transmit streamer
            // floating complex single precision - CPU
            // Complex-valued signed 16-bit integer data - OTW
            uhd::stream_args_t stream_args("fc32","sc16"); 
            uhd::tx_streamer::sptr tx_stream= tx_usrp->get_tx_stream(stream_args);


            // allocate a buffer and fill with single values
            int samplesPerBuffer=tx_stream->get_max_num_samps()*10;
            std::vector<std::complex<float>>  buff(samplesPerBuffer*10000,std::complex<float>{0.8, 0.0});
            //std::fill(buff.begin(), buff.end(), std::complex<float>{0.8, 0.0});
            std::vector<std::complex<float>*> pBuffs(1, &buff.front());
            

                
            // setup the metadata flags
            uhd::tx_metadata_t md;
            md.start_of_burst = true;
            md.end_of_burst   = false;
            md.has_time_spec  = true;
            md.time_spec = uhd::time_spec_t(2.0); // give us 0.5 seconds to fill the tx buffers

            // reset usrp time to prepare for transmit/receive
            std::cout << boost::format("Setting device timestamp to 0...") << std::endl;
            tx_usrp->set_time_now(uhd::time_spec_t(1.0));

            //send
            int i=5;
            while(i>0){
                tx_stream->send(pBuffs,buff.size(),md);
                i--;
                std::cout<<i<<std::endl;
            }

            md.end_of_burst = true;
            tx_stream->send("", 0, md);
            return 0;
        }


        int tx_waveform_from_file(uhd::usrp::multi_usrp::sptr tx_usrp, std::string precision, std::string waveformFilename, std::string outputFile, double secondsInFuture)
        {
            // Read in file 
            // TODO: assuming csv for now
            std::cout << "Reading in: " << waveformFilename << std::endl;


            std::vector<std::complex<double>> buffers = utils::read_in_complex_csv(waveformFilename);

            //set up transmit streamer
            uhd::stream_args_t stream_args("fc64","sc16");
            uhd::tx_streamer::sptr tx_stream= tx_usrp->get_tx_stream(stream_args);
            
            uhd::tx_metadata_t md;
            md.has_time_spec=true;
            md.time_spec      =  uhd::time_spec_t(secondsInFuture);
            md.start_of_burst=true;

            // reset usrp time to prepare for transmit/receive
            std::cout << boost::format("Setting device timestamp to 0...") << std::endl;
            tx_usrp->set_time_now(uhd::time_spec_t(0.0)); // TODO: use config

            hardware::transmitDoublesAtTime(tx_usrp, buffers, secondsInFuture, tx_stream, md);

            return 0;
        }
    }
} // namespace tests