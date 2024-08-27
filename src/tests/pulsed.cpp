#include "tests.hpp"
#include "utils.hpp"

#include <uhd/usrp/multi_usrp.hpp>
#include <iostream>
#include <fstream>
#include <filesystem>


namespace tests
{
    namespace pulsed
    {
        void tx_rx_pulsed(uhd::usrp::multi_usrp::sptr usrp, config::usrp_config usrp_config)
        {
            std::string waveformFile = usrp_config.get_waveform_filename();
            std::string outputFile = "outputs/pulsed_test"; // todo
            int numReps = 30; // TODO: parametrize


            // reset usrp time
            std::cout << boost::format("Setting device timestamp to 0...") << std::endl;
            usrp->set_time_now(uhd::time_spec_t(0.0));

            // same time, should be synced
            double secondsInFuture = usrp_config.get_tx_start_time();
            double settlingTime = usrp_config.get_rx_start_time();

            // transmit streamer
            uhd::stream_args_t stream_args("fc64","sc16");
            uhd::tx_streamer::sptr tx_stream= usrp->get_tx_stream(stream_args);

            // tx metadata
            uhd::tx_metadata_t tx_md;
            tx_md.has_time_spec=true;
            tx_md.time_spec      =  uhd::time_spec_t(secondsInFuture);
            tx_md.start_of_burst=true;
            tx_md.end_of_burst=false;

            // Read in file to transmit
            // TODO: assuming csv for now
            std::cout << "Reading in: " << waveformFile << std::endl;
            std::vector<std::complex<double>> tx_buffers = utils::read_in_complex_csv(waveformFile);
            size_t maxTransmitSize=tx_stream->get_max_num_samps();
            size_t fullTxBufferLength=tx_buffers.size();


            // receive streamer
            int num_requested_samples = usrp_config.get_num_samples(); 
            std::vector<size_t> rx_channel_nums(0); //SBX will be set up to only have 1 receive channel
            stream_args.channels             = rx_channel_nums;
            uhd::rx_streamer::sptr rx_stream = usrp->get_rx_stream(stream_args);
            size_t samps_per_buff=rx_stream->get_max_num_samps();// this overrwrites functions arguments. should not be allowed to stay here long
            std::cout<<"Max Receive Buffer Size: "<<samps_per_buff<<"\n";
            std::cout<<"Set number of requested samples: "<< num_requested_samples << std::endl; 
            // allocate buffers to receive with samples (one buffer per channel)
            std::vector<std::complex<double>> sampleBuffer(samps_per_buff);
            // creating a pointer to sample buffer
            std::complex<double>* psampleBuffer = &sampleBuffer[0];
            // creating one big buffer for writing to disk
            std::vector<double> fileBuffers(tx_buffers.size()*2);




            // setup streaming command for receive
            uhd::stream_cmd_t rx_stream_cmd=uhd::stream_cmd_t::STREAM_MODE_NUM_SAMPS_AND_DONE;
            rx_stream_cmd.num_samps  = tx_buffers.size();
            rx_stream_cmd.stream_now = false;
            double pulseLengthSecs = tx_buffers.size() / usrp_config.get_tx_rate();
            rx_stream_cmd.time_spec  = uhd::time_spec_t(settlingTime) + uhd::time_spec_t(2*pulseLengthSecs);
            std::cout << "Tx initial burst start time: " << tx_md.time_spec.get_real_secs() << std::endl;
            std::cout << "Rx initial burst start time: " << rx_stream_cmd.time_spec.get_real_secs() << std::endl;     
            rx_stream->issue_stream_cmd(rx_stream_cmd); // in future, 1 pulse length ahead of Tx

            size_t numSamplesReceivedTotal=0;
            size_t numSamplesReceivedFrame=0;
            uhd::rx_metadata_t rxMetaData;

            // TODO: remove all frames first?
            int counter = 0;

            // transmit for waveform size, then receive for waveform size. until num requested samples received
            while (counter < numReps)
            {
                // transmit one pulse
                int time = usrp->get_time_now().get_real_secs();
                std::cout << "TX pulse " << time << std::endl;
                tx_md.start_of_burst=true;
                tx_md.end_of_burst=false;
                fullTxBufferLength = tx_buffers.size();
                if(fullTxBufferLength<=maxTransmitSize)
                {
                    //std::cout << "Full buffer length <= max transmit size" << std::endl;
                    std::vector<std::complex<double>*> pBuffs(1,&tx_buffers.front());
                    tx_stream->send(pBuffs,tx_buffers.size(),tx_md);
                }else
                {
                    //std::cout << "Full buffer length >= max transmit size" << std::endl;    
                    size_t numSent=0;
                    // Split buffer into segments based on the max transmit size
                    while (numSent<fullTxBufferLength)
                    {
                        size_t smallBufferSize=fullTxBufferLength-numSent;
                        // if not last
                        if(smallBufferSize>maxTransmitSize){
                            smallBufferSize=maxTransmitSize;
                        }
                        std::vector<std::complex<double>> smallbuffer(tx_buffers.begin()+numSent,tx_buffers.begin()+numSent+smallBufferSize);
                        std::vector<std::complex<double>*> pBuffs(1,&smallbuffer.front());
                        tx_stream->send(pBuffs,smallbuffer.size(),tx_md);
                        numSent+=smallBufferSize;
                        
                        tx_md.has_time_spec=false; //dont want subsequent packets to wait
                        tx_md.start_of_burst=false;
                    }
                    
                    // Send end of burst packet
                    tx_md.start_of_burst=false;
                    tx_md.end_of_burst=true;
                    tx_stream->send("",0,tx_md);
                }
                tx_md.has_time_spec=true;
                tx_md.time_spec = usrp->get_time_now() + uhd::time_spec_t(pulseLengthSecs);


        


                // receive for one pulse (file length)
                time = usrp->get_time_now().get_real_secs();
                std::cout << "RX pulse " << time << std::endl;
                numSamplesReceivedFrame = 0;
                size_t numNewSamples = 1;
                //while (numSamplesReceivedFrame < tx_buffers.size())
                while (numNewSamples != 0)
                {
                    
                    double samplesForThisBlock=tx_buffers.size()-numSamplesReceivedFrame;

                    // if not last block
                    if (samplesForThisBlock>=samps_per_buff)
                    {
                        samplesForThisBlock=samps_per_buff;
                    }
                    
                    numNewSamples=rx_stream->recv(psampleBuffer,samplesForThisBlock,rxMetaData);
                    //std::cout << "Recv samples for this frame " << numSamplesReceivedFrame << std::endl;

                    double* pfileBuffers = fileBuffers.data() + numSamplesReceivedFrame*2; // destination to copy to
                    std::complex<double>* pSource = psampleBuffer; // received samples
                    std::memcpy(pfileBuffers, pSource, numNewSamples * sizeof(double) * 2);

                    //increment num samples receieved
                    numSamplesReceivedFrame+=numNewSamples;
                }
                numSamplesReceivedTotal += numSamplesReceivedFrame;
                std::cout << numSamplesReceivedFrame << std::endl;

                // write to frame
                // increment file counter (start at 0 for easier processing)
                std::string frameFilename = "frames/frame" + std::to_string(counter) + ".bin";
                counter++;
                std::ofstream outFileApp(frameFilename, std::ofstream::app);
                outFileApp.write(reinterpret_cast<char*>(fileBuffers.data()),2*numSamplesReceivedFrame*sizeof(double));
                outFileApp.close();

                uhd::stream_cmd_t rx_stream_cmd=uhd::stream_cmd_t::STREAM_MODE_NUM_SAMPS_AND_DONE;
                rx_stream_cmd.num_samps  = tx_buffers.size();
                rx_stream_cmd.stream_now = false;
                rx_stream_cmd.time_spec = usrp->get_time_now() + uhd::time_spec_t(2*pulseLengthSecs);
                rx_stream->issue_stream_cmd(rx_stream_cmd);

            }
        }
    }
}