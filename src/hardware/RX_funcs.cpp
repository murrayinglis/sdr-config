#include "hardware.hpp"
#include "utils.hpp"

#include <uhd/usrp/multi_usrp.hpp>
#include <iostream>
#include <fstream>
#include <filesystem>

using namespace std::chrono_literals;


// TODO: this is only todo with streaming now
// Rename
namespace hardware{
    static bool stop_signal_called = false; // not used at all...

    /**
     * RX FUNCS
     * -------------------------------------------------
     */
    std::vector<std::complex<double>> captureDoubles(uhd::usrp::multi_usrp::sptr rx_usrp,size_t numSamples,double settling_time){
        // these should be constants
        std::string cpu_format="fc64"; // function of doubles
        std::string wire_format="sc16"; // https://files.ettus.com/manual/structuhd_1_1stream__args__t.html#a0ba0e946d2f83f7ac085f4f4e2ce9578
        std::vector<size_t> rx_channel_nums(0); // SBX only has 1 rx channel
        
        // create a receive streamer
        uhd::stream_args_t stream_args(cpu_format, wire_format);
        stream_args.channels             = rx_channel_nums;
        uhd::rx_streamer::sptr rx_stream = rx_usrp->get_rx_stream(stream_args);
        size_t samps_per_buff=rx_stream->get_max_num_samps();

        // create totalVector
        std::vector<std::complex<double>> entireSample;
        entireSample.reserve(numSamples);

        // allocate buffers to receive with samples (one buffer per channel)
        std::vector<std::complex<double>> sampleBuffer(samps_per_buff);

        // creating a pointer to sample buffer
        std::complex<double>* psampleBuffer = &sampleBuffer[0];


        // setup streaming
        uhd::stream_cmd_t stream_cmd=uhd::stream_cmd_t::STREAM_MODE_NUM_SAMPS_AND_DONE;
        stream_cmd.num_samps  = numSamples;
        stream_cmd.stream_now = false;
        stream_cmd.time_spec  = uhd::time_spec_t(settling_time);
        rx_stream->issue_stream_cmd(stream_cmd);


        // 
        size_t numSamplesReceived=0;
        uhd::rx_metadata_t rxMetaData;
        

        while (numSamplesReceived<numSamples){
            double samplesForThisBlock=numSamples-numSamplesReceived;
            if (samplesForThisBlock>samps_per_buff){
                samplesForThisBlock=samps_per_buff;
            }
            
            size_t numNewSamples=rx_stream->recv(psampleBuffer,samplesForThisBlock,rxMetaData);

            //append received data to rest of buffer
            entireSample.insert(entireSample.begin()+numSamplesReceived, sampleBuffer.begin(), sampleBuffer.begin()+numNewSamples);
            //increment num samples receieved
            numSamplesReceived+=numNewSamples;
        }
        return entireSample;
    } // captureDoubles




    void recv_to_file_doubles(uhd::usrp::multi_usrp::sptr usrp,
        uhd::rx_streamer::sptr rx_stream,
        const std::string& file,
        int num_requested_samples,
        double settling_time,
        bool storeMD)
    {


        size_t samps_per_buff=rx_stream->get_max_num_samps();// this overrwrites functions arguments. should not be allowed to stay here long
        std::cout<<"Max Receive Buffer Size: "<<samps_per_buff<<"\n";
        std::cout<<"Set number of requested samples: "<< num_requested_samples << std::endl; 
        // confirm that we are indeed only using 1 channel
        //unsigned int numRxChannels = rx_stream->get_num_channels();
        //std::cout << "Set up HARDWARE stream. Num input channels: " << numRxChannels << std::endl;
        //std::cout << usrp->get_pp_string();


        // setup streaming
        uhd::stream_cmd_t stream_cmd=uhd::stream_cmd_t::STREAM_MODE_NUM_SAMPS_AND_DONE;
        stream_cmd.num_samps  = num_requested_samples;
        stream_cmd.stream_now = false;
        //stream_cmd.time_spec  = uhd::time_spec_t(settling_time);
        stream_cmd.time_spec  = usrp->get_time_now() + uhd::time_spec_t(settling_time);
        std::cout << "\nActual RX time spec when stream command is issued: " << stream_cmd.time_spec.get_real_secs() << std::endl; //...
        rx_stream->issue_stream_cmd(stream_cmd);


        //
        uhd::rx_metadata_t rxMetaData;
        // metadata file (good idea Stephen)
        if (storeMD){
            std::ofstream metaDataFileStream;
            std::string metaDataFile=file+"_rx_metadata.txt";
            metaDataFileStream.open(metaDataFile);

            auto timenow = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
	        metaDataFileStream << boost::format("System time at start: %s") % ctime(&timenow) << std::endl;
            metaDataFileStream << boost::format("Clock Reference: %s") % usrp->get_clock_source(0) << std::endl;
            // TODO: finish this up


        }
        
        std::string outputFileName(file+".bin");
        std::ofstream outFileClear(outputFileName.c_str(), std::ios::binary | std::ios::trunc); // overwrite file
        outFileClear.close();

        //std::cout << utils::getCurrentEpochTime_ns() << std::endl;
        uhd::time_spec_t time_now = usrp->get_time_now();
        //std::cout << "RX time: " << time_now.get_real_secs() << std::endl;


        // allocate buffers to receive with samples (one buffer per channel)
        std::vector<std::complex<double>> sampleBuffer(samps_per_buff);
        // creating a pointer to sample buffer
        std::complex<double>* psampleBuffer = sampleBuffer.data();
        // creating one big buffer for writing to disk
        std::vector<double> fileBuffers(2*num_requested_samples);
        int numSamplesReceived=0;
        
        

        // write to file when ALL samples received
        bool time_spec_reached = false;
        while (numSamplesReceived<num_requested_samples){
            double samplesForThisBlock=num_requested_samples-numSamplesReceived;
            if (samplesForThisBlock>samps_per_buff){
                samplesForThisBlock=samps_per_buff;
            }
            
            size_t numNewSamples=rx_stream->recv(psampleBuffer,samplesForThisBlock,rxMetaData, 0.5);
            if (numNewSamples > 0 && !time_spec_reached)
            {
                std::cout << "\nRX time spec reached, receiving started: " << usrp->get_time_now().get_real_secs() << std::endl;
                time_spec_reached = true;
            }

            double* pfileBuffers = fileBuffers.data() + numSamplesReceived*2; // destination to copy to
            std::complex<double>* pSource = psampleBuffer; // received samples
            std::memcpy(pfileBuffers, pSource, numNewSamples * sizeof(double) * 2);

            //increment num samples receieved
            numSamplesReceived+=numNewSamples;
            //std::cout << numSamplesReceived << std::endl;
        }

        // write to file when all samples received
        std::ofstream outFileApp(outputFileName, std::ofstream::app);
        outFileApp.write(reinterpret_cast<char*>(fileBuffers.data()),2*numSamplesReceived*sizeof(double));
        outFileApp.close();
        
       
    } //recv_to_file_doubles




} //namespace HARDWARE