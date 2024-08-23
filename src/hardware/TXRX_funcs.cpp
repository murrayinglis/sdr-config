#include "hardware.hpp"
#include "utils.hpp"

#include <uhd/usrp/multi_usrp.hpp>
#include <fstream>

namespace hardware
{
        std::atomic_bool sync_flag(false);

        void tx_doublesAtTimeSpec(uhd::usrp::multi_usrp::sptr usrp, 
        std::vector<std::complex<double>> buffers, 
        double secondsInFuture, 
        uhd::tx_streamer::sptr tx_stream, 
        uhd::tx_metadata_t md)
        {
            //uhd::time_spec_t time_now = tx_usrp->get_time_now();
            //std::cout << time_now.get_real_secs() << std::endl;
            //set up transmit streamer BEFORE
            //uhd::stream_args_t stream_args("fc64","sc16");
            //uhd::tx_streamer::sptr tx_stream= tx_usrp->get_tx_stream(stream_args);
            
            //uhd::tx_metadata_t md;
            md.has_time_spec=true;
            md.time_spec      =  uhd::time_spec_t(secondsInFuture);
            md.start_of_burst=true;
            md.end_of_burst=false;



            size_t maxTransmitSize=tx_stream->get_max_num_samps(); //not entirely sure where this comes from
            //std::cout<<"Max Transmit Buffer Size: "<<maxTransmitSize<<"\n";
            size_t fullBufferLength=buffers.size();

            // TODO: alert with mtx
            while(!sync_flag.load())
            {

            }

            std::cout << "TX started at: " << usrp->get_time_now().get_real_secs() << std::endl;
            for (int i = 0; i < 5; i++)
            {
            if(fullBufferLength<=maxTransmitSize){
                //std::cout << "Full buffer length <= max transmit size" << std::endl;
                std::vector<std::complex<double>*> pBuffs(1,&buffers.front());
                tx_stream->send(pBuffs,buffers.size(),md);
                md.end_of_burst=true;
                tx_stream->send("",0,md);
                return;
            }else{
                // wait for recv to start

                //std::cout << "Full buffer length >= max transmit size" << std::endl;    
                size_t numSent=0;
                // Split buffer into segments based on the max transmit size
                while (numSent<fullBufferLength)
                {
                    size_t smallBufferSize=fullBufferLength-numSent;
                    if(smallBufferSize>maxTransmitSize){
                        smallBufferSize=maxTransmitSize;
                    }
                    std::vector<std::complex<double>> smallbuffer(buffers.begin()+numSent,buffers.begin()+numSent+smallBufferSize);
                    std::vector<std::complex<double>*> pBuffs(1,&smallbuffer.front());
                    tx_stream->send(pBuffs,smallbuffer.size(),md, 0.5);
                    numSent+=smallBufferSize;
                    
                    md.has_time_spec=false; //dont want subsequent packets to wait
                    md.start_of_burst=false;
                }

                // Send end of burst packet
                md.end_of_burst=true;
                tx_stream->send("",0,md);
            }
            }
            std::cout << "TX end at: " << usrp->get_time_now().get_real_secs() << std::endl;
        }


        void rx_doublesAtTimeAlert(uhd::usrp::multi_usrp::sptr usrp, 
        int num_requested_samples,
        double rx_start_time, 
        uhd::rx_streamer::sptr rx_stream)
        {

            size_t samps_per_buff=rx_stream->get_max_num_samps();// this overrwrites functions arguments. should not be allowed to stay here long
            std::cout<<"Max Receive Buffer Size: "<<samps_per_buff<<"\n";
            std::cout<<"Set number of requested samples: "<< num_requested_samples << std::endl; 
            // confirm that we are indeed only using 1 channel
            //unsigned int numRxChannels = rx_stream->get_num_channels();
            //std::cout << "Set up HARDWARE stream. Num input channels: " << numRxChannels << std::endl;
            //std::cout << usrp->get_pp_string();


            // allocate buffers to receive with samples (one buffer per channel)
            std::vector<std::complex<double>> sampleBuffer(samps_per_buff);


            // creating a pointer to sample buffer
            std::complex<double>* psampleBuffer = &sampleBuffer[0];

            // creating one big buffer for writing to disk
            std::vector<double> fileBuffers(2*num_requested_samples);


            // setup streaming
            uhd::stream_cmd_t stream_cmd=uhd::stream_cmd_t::STREAM_MODE_NUM_SAMPS_AND_DONE;
            stream_cmd.num_samps  = num_requested_samples;
            stream_cmd.stream_now = false;
            //stream_cmd.time_spec  = uhd::time_spec_t(settling_time);
            stream_cmd.time_spec  = usrp->get_time_now() + uhd::time_spec_t(rx_start_time);
            std::cout << "\nActual RX time spec when stream command is issued: " << stream_cmd.time_spec.get_real_secs() << std::endl; //...
            rx_stream->issue_stream_cmd(stream_cmd);



            //
            double numSamplesReceived=0;
            uhd::rx_metadata_t rxMetaData;

            std::string file = "outputs/latency_test";
            std::string outputFileName(file+".bin");
            std::ofstream outFile(outputFileName.c_str(), std::ios::binary | std::ios::trunc); // overwrite file
            outFile.close();


            bool time_spec_reached = false;
            while (numSamplesReceived<num_requested_samples){
                double samplesForThisBlock=num_requested_samples-numSamplesReceived;
                if (samplesForThisBlock>samps_per_buff){
                    samplesForThisBlock=samps_per_buff;
                }
                
                size_t numNewSamples=rx_stream->recv(psampleBuffer,samplesForThisBlock,rxMetaData);
                if (numNewSamples > 0 && !time_spec_reached)
                {
                    std::cout << "\nRX time spec reached, TX flag set: " << usrp->get_time_now().get_real_secs() << std::endl;
                    sync_flag.store(true);
                    time_spec_reached = true;
                }

                //copy data to filebuffer
                double* pDestination = fileBuffers.data();
                std::complex<double>* pSource=psampleBuffer;
                memcpy(pDestination,pSource,2*numNewSamples*sizeof(double));

                //write filebuffer to file
                std::ofstream outFile(outputFileName, std::ofstream::app);
                outFile.write(reinterpret_cast<char*>(fileBuffers.data()),2*numNewSamples*sizeof(double));
                outFile.close();
                //increment num samples receieved
                numSamplesReceived+=numNewSamples;

            }
            std::cout << "RX end at: " << usrp->get_time_now().get_real_secs() << std::endl;


        }


}