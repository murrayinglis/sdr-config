#include "tests.hpp"
#include <iostream>
#include <fstream>
#include <boost/thread/thread.hpp>
#include <uhd/usrp/multi_usrp.hpp>

namespace tests
{
    namespace pulsed
    {
        void rx_worker(double settlingTime,
                       int numSamplesRequested,
                       uhd::rx_streamer::sptr rx_stream,
                       uhd::usrp::multi_usrp::sptr usrp,
                       std::string outputFilename)
        {
            // 1. setup streamer metadata
            uhd::rx_metadata_t md;
            md.has_time_spec = true;
            md.time_spec = uhd::time_spec_t(settlingTime);


            // 2. check for samples exceed RX buffers max
            // NOTE: for my USRP N200, max is 363
            int samps_per_buff = numSamplesRequested;
            int max_samps = rx_stream->get_max_num_samps();
            int num_commands = 1;
            // Generate multiple rx commands if so
            std::cout << numSamplesRequested 
            << " samples requested :: " 
            << max_samps 
            << " allowed at maximum per rx buffer." 
            << std::endl;
            if (max_samps < numSamplesRequested)
            {   
                samps_per_buff = numSamplesRequested;
                num_commands = numSamplesRequested / max_samps + 1;
                std::cout << num_commands << " rx commands required per pulse." << std::endl;
            }
            // setup streaming command
            uhd::stream_cmd_t stream_cmd=uhd::stream_cmd_t::STREAM_MODE_NUM_SAMPS_AND_DONE;
            stream_cmd.num_samps  = samps_per_buff;
            stream_cmd.stream_now = false;
            //stream_cmd.time_spec  = uhd::time_spec_t(settling_time);
            stream_cmd.time_spec  = usrp->get_time_now() + uhd::time_spec_t(settlingTime);
            std::cout << "\nActual RX time spec when stream command is issued: " << stream_cmd.time_spec.get_real_secs() << std::endl; //...
            rx_stream->issue_stream_cmd(stream_cmd);

            // 3. Gen RX buffer pointers
            // allocate buffers to receive with samples (one buffer per channel)
            std::vector<std::complex<double>> sampleBuffer(samps_per_buff);
            // creating a pointer to sample buffer
            std::complex<double>* psampleBuffer = sampleBuffer.data();
            // creating one big buffer for writing to disk
            std::vector<double> fileBuffers(2*numSamplesRequested);
            int numSamplesReceived=0;
            
            

            // This will receive until the buffer is filled with the number of requested samples
            bool time_spec_reached = false;
            while (numSamplesReceived<numSamplesRequested){
                double samplesForThisBlock=numSamplesRequested-numSamplesReceived;
                if (samplesForThisBlock>samps_per_buff){
                    samplesForThisBlock=samps_per_buff;
                }
                
                size_t numNewSamples=rx_stream->recv(psampleBuffer,samplesForThisBlock,md, 0.5);
                if (numNewSamples > 0 && !time_spec_reached)
                {
                    std::cout << "\nRX time spec reached, receiving started: " << usrp->get_time_now().get_real_secs() << std::endl;
                    time_spec_reached = true;
                }
                else if (!time_spec_reached)
                {
                    std::cout << "Time now: " << usrp->get_time_now().get_real_secs() << " :: RX is waiting for time spec..." << std::endl;
                }

                double* pfileBuffers = fileBuffers.data() + numSamplesReceived*2; // destination to copy to
                std::complex<double>* pSource = psampleBuffer; // received samples
                std::memcpy(pfileBuffers, pSource, numNewSamples * sizeof(double) * 2);

                //increment num samples receieved
                numSamplesReceived+=numNewSamples;

            }

            // write to file when all samples received
            std::ofstream outFileApp(outputFilename, std::ofstream::app);
            outFileApp.write(reinterpret_cast<char*>(fileBuffers.data()),2*numSamplesReceived*sizeof(double));
            outFileApp.close();



            // clear buffers
            std::cout << "Writing to RX file." << std::endl;
        }

        void tx_worker(double secondsInFuture,
                       uhd::tx_streamer::sptr tx_stream,
                       uhd::usrp::multi_usrp::sptr usrp,
                       std::vector<std::complex<double>> buffers,
                       int reps)
        {
            // 1. Setup streamer metadat
            uhd::tx_metadata_t md;
            md.has_time_spec=true;
            md.time_spec      =  uhd::time_spec_t(secondsInFuture);
            md.start_of_burst=true;

            // 2. Transmit initial burst

            // 3. Check for STOP called
            int max_samps = tx_stream->get_max_num_samps();
            std::cout 
            << "Sending "
            << buffers.size()
            << " buffers per pulse :: "
            << max_samps 
            << " samples allowed at maximum per tx buffer." 
            << std::endl;
            size_t fullBufferLength=buffers.size();

            // tx if not
            // transmit until told not to
            bool tx_started = false;
            // only increment rep when tx occurs, check tx md to ensure
            int idx = 0;

            while (idx < reps)
            {     
                bool time_spec_reached = false; 
                md.start_of_burst=true;
                md.end_of_burst=false;
                fullBufferLength = buffers.size();

                
                if(fullBufferLength<=max_samps)
                {
                    //std::cout << "Full buffer length <= max transmit size" << std::endl;
                    std::vector<std::complex<double>*> pBuffs(1,&buffers.front());
                    tx_stream->send(pBuffs,buffers.size(),md, 0.5);
                    md.end_of_burst=true;
                    tx_stream->send("",0,md);
                }else
                {
                    //std::cout << "Full buffer length >= max transmit size" << std::endl;    
                    size_t numSent=0;
                    // Split buffer into segments based on the max transmit size
                    while (numSent<fullBufferLength)
                    {
                        size_t smallBufferSize=fullBufferLength-numSent;
                        if(smallBufferSize>max_samps){
                            smallBufferSize=max_samps;
                        }
                        std::vector<std::complex<double>> smallbuffer(buffers.begin()+numSent,buffers.begin()+numSent+smallBufferSize);
                        std::vector<std::complex<double>*> pBuffs(1,&smallbuffer.front());
                        size_t numSentNow = tx_stream->send(pBuffs,smallbuffer.size(),md);
                        if (numSentNow > 0)
                        {
                            numSent+=smallBufferSize;
                        }
                        else
                        {
                            std::cout << "Time now: " << usrp->get_time_now().get_real_secs() << " :: TX is waiting for time spec..." << std::endl;
                        }
                        if (numSentNow > 0 && !time_spec_reached)
                        {
                            std::cout << "\nTX time spec reached, transmitting started: " << usrp->get_time_now().get_real_secs() 
                            << ". Rep: "
                            << idx
                            << std::endl;
                            time_spec_reached = true;
                        }


                        
                        md.has_time_spec=false; //dont want subsequent packets to wait
                        md.start_of_burst=false;
                    }

                    // Send end of burst packet
                    md.end_of_burst=true;
                    tx_stream->send("",0,md);
                }

                idx++;
            }
            std::cout << "Finished TX reps" << std::endl;


            // 4. clear buffers 
        }

        void pulsed_pipeline(uhd::usrp::multi_usrp::sptr usrp, config::usrp_config usrp_config)
        {
            double settlingTime = usrp_config.get_rx_start_time();
            double secondsInFuture = usrp_config.get_tx_start_time();
            double numSampsRequested = usrp_config.get_num_samples();
            std::string outputFilename = "frames/frame.bin";
            int reps = 100;
            std::vector<std::complex<double>> buffers(12000,std::complex<float>{0.8, 0.0});

            // reset usrp time to prepare for transmit/receive
            std::cout << boost::format("Setting device timestamp to 0...") << std::endl;
            usrp->set_time_now(uhd::time_spec_t(0.0));

            //set up transmit streamer
            uhd::stream_args_t stream_args("fc64","sc16");
            std::vector<size_t> tx_channel_nums(0);
            stream_args.channels = tx_channel_nums;
            uhd::tx_streamer::sptr tx_stream= usrp->get_tx_stream(stream_args);

            // create a receive streamer - use same cpu and wire format
            std::vector<size_t> rx_channel_nums(0); 
            stream_args.channels             = rx_channel_nums;
            uhd::rx_streamer::sptr rx_stream = usrp->get_rx_stream(stream_args);\


            boost::thread_group thread_group;
            thread_group.create_thread(boost::bind(&rx_worker, settlingTime, numSampsRequested, rx_stream, usrp, outputFilename));
            thread_group.create_thread(boost::bind(&tx_worker, secondsInFuture, tx_stream, usrp, buffers, reps));

            thread_group.join_all();
        }
    }
}