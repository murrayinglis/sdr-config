#include "tests.hpp"
#include "utils.hpp"

#include <uhd/usrp/multi_usrp.hpp>
#include <uhd/utils/thread.hpp>
#include <uhd/utils/safe_main.hpp>

#include <iostream>
#include <fstream>
#include <filesystem>

namespace tests
{
    namespace pulsed
    {
        void tx_rx_pulsed(uhd::usrp::multi_usrp::sptr usrp, size_t numSamples, std::string outputFile, std::string waveformFile)
        {


            // reset usrp time
            std::cout << boost::format("Setting device timestamp to 0...") << std::endl;
            usrp->set_time_now(uhd::time_spec_t(0.0));

            // same time, should be synced
            double secondsInFuture = 0.1;
            double settlingTime = 0.1;

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
            //std::vector<std::complex<double>>  zeros(fullTxBufferLength,std::complex<double>{0.0, 0.0});

            // receive streamer
            int num_requested_samples = 500000; // TODO: parametrize
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
            std::vector<double> fileBuffers(2*num_requested_samples);


            // setup streaming command for receive
            uhd::stream_cmd_t rx_stream_cmd=uhd::stream_cmd_t::STREAM_MODE_NUM_SAMPS_AND_DONE;
            rx_stream_cmd.num_samps  = num_requested_samples;
            rx_stream_cmd.stream_now = false;
            //stream_cmd.time_spec  = uhd::time_spec_t(settling_time);
            rx_stream_cmd.time_spec  = usrp->get_time_now() + uhd::time_spec_t(settlingTime);
            rx_stream->issue_stream_cmd(rx_stream_cmd); // now or later?

            double numSamplesReceivedTotal=0;
            double numSamplesReceivedPulse=0;
            uhd::rx_metadata_t rxMetaData;
            std::string outputFileName(outputFile+".bin");
            std::ofstream outFile(outputFileName.c_str(), std::ios::binary | std::ios::trunc); // overwrite file
            outFile.close();

            // transmit for waveform size, then receive for waveform size. until num requested samples received
            while (numSamplesReceivedTotal<num_requested_samples)
            {
                // transmit one pulse
                std::cout << "TX pulse" << std::endl;
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

        


                // receive for one pulse (file length)
                std::cout << "RX pulse" << std::endl;
                numSamplesReceivedPulse = 0;
                size_t numNewSamples = 1;
                //while (numSamplesReceivedPulse < tx_buffers.size())
                while (numNewSamples != 0)
                {
                    
                    double samplesForThisBlock=tx_buffers.size()-numSamplesReceivedPulse;

                    // if not last block
                    if (samplesForThisBlock>=samps_per_buff)
                    {
                        samplesForThisBlock=samps_per_buff;
                    }
                    
                    numNewSamples=rx_stream->recv(psampleBuffer,samplesForThisBlock,rxMetaData);
                    //std::cout << "Recv samples for this pulse " << numSamplesReceivedPulse << std::endl;


                    //copy data to filebuffer
                    double* pDestination = fileBuffers.data();
                    std::complex<double>* pSource=psampleBuffer;
                    memcpy(pDestination,pSource,2*numNewSamples*sizeof(double));

                    //write filebuffer to file
                    std::ofstream outFile(outputFileName, std::ofstream::app);
                    outFile.write(reinterpret_cast<char*>(fileBuffers.data()),2*numNewSamples*sizeof(double));
                    outFile.close();
                    //increment num samples receieved
                    numSamplesReceivedPulse+=numNewSamples;
                }
                numSamplesReceivedTotal += numSamplesReceivedPulse;

                // NOTE: this is !!NOT!! how pulsed radar works, receive pulse is meant to be much longer
                // for understanding sake, append pulse length of 0s to see time not receiving
                //std::ofstream outFile(outputFileName, std::ofstream::app);
                //outFile.write(reinterpret_cast<char*>(zeros.data()), zeros.size() * sizeof(double) * 2);
                //outFile.close();

            }
        }

        void pulsed_cw(uhd::usrp::multi_usrp::sptr usrp)
        {
            std::string waveformFile = "sweep.csv";
            std::vector<std::complex<double>> tx_buffers = utils::read_in_complex_csv(waveformFile);


        }

        void rxFunc(
            uhd::usrp::multi_usrp::sptr usrp,
            uhd::rx_streamer::sptr rx_stream,
            double freq,
            double time,
            int numSamps
        )
        {

            uhd::set_thread_priority_safe();
            double interval = 10e-3; // TODO: parameter in config

            std::vector<std::vector<std::complex<double>>> buffs; // a vector of units of the doubles
            buffs.resize(result->size()); // TODO
            std::vector<std::complex<double>*> Rxbuff; // a vector of pointers to(arrays of) complex doubles
            for(int i = 0; i < result->size(); i++)
            {
                buffs[i].resize(numSamps);
                Rxbuff.push_back(&buffs[i].front());
            }

            uhd::rx_metadata_t rxmd;

            uhd::time_spec_t theTime;
            theTime = usrp->get_time_now();
            uhd::stream_cmd_t cmd(uhd::stream_cmd_t::STREAM_MODE_NUM_SAMPS_AND_DONE);
            cmd.num_samps = numSamps;
            cmd.time_spec = time;
            cmd.stream_now = false;//dont stream now, use time spec
            if(time < theTime.get_real_secs())
            {
                std::cout << "timing problem!" << std::endl;
                std::cout <<"RXTIMESPEC "<< cmd.time_spec.get_real_secs() << std::endl;
                std::cout <<"USRP time(RxFunc) "<< theTime.get_real_secs()<< std::endl;
                std::cout <<"USRP Capture Time(RxFunc) "<< time<< std::endl;
            }

            for(int i = 0; i < result->size();i++)
            {
                rx_stream->issue_stream_cmd(cmd);
                num_rx_samps = rx_stream->recv(Rxbuff[i], numSamps, rxmd,1,false); 
                // cout <<"rx @ t= "<< rxmd.time_spec.get_real_secs()<< endl;
                cmd.time_spec = cmd.time_spec.get_real_secs()+interval;
                if(rxmd.error_code != 0)
                {
                    std::cout << "rxmd. "<< rxmd.time_spec.get_real_secs()<<std::endl;
                    std::cout << "Stored "<< num_rx_samps << " samples."<< std::endl;
                    std::cout << "Rx md error code: "<< rxmd.error_code << " RxMD errstr"<< rxmd.strerror()<< std::endl;
                }
            }   


            for(int i = 0; i < result->size();i++)
            {
                (*result)[i].resize(numSamps);
                if
                {
                    (usrp->get_master_clock_rate()>61.44e6)(*result)[i].Fs = 100e6;
                }
                else
                {
                     (*result)[i].Fs = 30e6;
                }
                for(int j = 0; j < numSamps; j ++)
                {
                    (*result)[i][j] = std::complex<double>(buffs[i][j].real(),buffs[i][j].imag());
                }
            }


            if(rxmd.error_code != 0)
            {
                std::cout << "rxmd. "<< rxmd.time_spec.get_real_secs()<<std::endl;
                std::cout << "Stored "<< num_rx_samps << " samples."<< std::endl;
                std::cout << "Rx md error code: "<< rxmd.error_code << " RxMD errstr "<<
                rxmd.strerror()<< std::endl;
            }
            return
}