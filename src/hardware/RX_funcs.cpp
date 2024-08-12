#include "hardware.hpp"
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
    const std::string& file,
    int num_requested_samples,
    double settling_time,
    bool storeMD){

        std::string cpu_format="fc64";
        std::string wire_format="sc16";
        std::vector<size_t> rx_channel_nums(0); //SBX will be set up to only have 1 receive channel
        
        // create a receive streamer
        uhd::stream_args_t stream_args(cpu_format, wire_format);
        stream_args.channels             = rx_channel_nums;
        uhd::rx_streamer::sptr rx_stream = usrp->get_rx_stream(stream_args);
        size_t samps_per_buff=rx_stream->get_max_num_samps();// this overrwrites functions arguments. should not be allowed to stay here long
        std::cout<<"Max Receive Buffer Size"<<samps_per_buff<<"\n";

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
        stream_cmd.time_spec  = uhd::time_spec_t(settling_time);
        rx_stream->issue_stream_cmd(stream_cmd);




        //
        double numSamplesReceived=0;
        uhd::rx_metadata_t rxMetaData;

        
        // metadata file (good idea Stephen)
        if (storeMD){
            std::ofstream metaDataFileStream;
            std::string metaDataFile=file+"_metadata.txt";
            metaDataFileStream.open(metaDataFile);

            auto timenow = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
	        metaDataFileStream << boost::format("System time at start: %s") % ctime(&timenow) << std::endl;
            metaDataFileStream << boost::format("Clock Reference: %s") % usrp->get_clock_source(0) << std::endl;
            // TODO: finish this up


        }
        
        std::string outputFileName(file+".bin");
        std::ofstream outFile(outputFileName.c_str(), std::ios::binary | std::ios::trunc);
        outFile.close();

        while (numSamplesReceived<num_requested_samples){
            double samplesForThisBlock=num_requested_samples-numSamplesReceived;
            if (samplesForThisBlock>samps_per_buff){
                samplesForThisBlock=samps_per_buff;
            }
            
            size_t numNewSamples=rx_stream->recv(psampleBuffer,samplesForThisBlock,rxMetaData);

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
       
    } //recv_to_file_doubles

    void recv_samples_to_file(uhd::usrp::multi_usrp::sptr usrp,
    const std::string& cpu_format,
    const std::string& wire_format,
    const std::vector<size_t>& channel_nums,
    const size_t total_num_channels,
    const std::string& file,
    size_t samps_per_buff,
    unsigned long long num_requested_samples,
    double& bw,
    std::mutex* recv_mutex,
    double time_requested,            
    bool stats,           
    bool null,                        
    bool enable_size_map,             
    bool continue_on_bad_packet,      
    const std::string& thread_prefix)
    {
        unsigned long long num_total_samps = 0;

        // create a receive streamer
        uhd::stream_args_t stream_args(cpu_format, wire_format);
        stream_args.channels             = channel_nums;
        uhd::rx_streamer::sptr rx_stream = usrp->get_rx_stream(stream_args);

        uhd::rx_metadata_t md;

        // Cannot use std::vector as second dimension type because recv will call
        // reinterpret_cast<char*> on each subarray, which is incompatible with
        // std::vector. Instead create new arrays and manage the memory ourselves
        // double samp type used - maybe change to complex
        std::vector<double*> buffs(rx_stream->get_num_channels());
        try {
            for (size_t ch = 0; ch < rx_stream->get_num_channels(); ch++) {
                buffs[ch] = new double[samps_per_buff];
            }
        } catch (std::bad_alloc& exc) {
            UHD_LOGGER_ERROR("UHD")
                << "Bad memory allocation. "
                "Try a smaller samples per buffer setting or free up additional memory";
            std::exit(EXIT_FAILURE);
        }

        /**
         * Filename output stuff
         * 
         */
        std::vector<std::ofstream> outfiles(rx_stream->get_num_channels());
        std::string filename;
        for (size_t ch = 0; ch < rx_stream->get_num_channels(); ch++) {
            if (not null) {
                if (rx_stream->get_num_channels() == 1) { // single channel
                    filename = file;
                } else { // multiple channels
                    // check if file extension exists
                    if (file.find('.') != std::string::npos) {
                        const std::string base_name = file.substr(0, file.find_last_of('.'));
                        const std::string extension = file.substr(file.find_last_of('.'));
                        filename = base_name + "_" + "ch" + std::to_string(channel_nums[ch])
                                + extension;
                    } else {
                        // file extension does not exist
                        filename = file + "_" + "ch" + std::to_string(channel_nums[ch]);
                    }
                }
                outfiles[ch].open(filename.c_str(), std::ofstream::binary);
            }
        }

        // setup streaming
        uhd::stream_cmd_t stream_cmd((num_requested_samples == 0)
                                        ? uhd::stream_cmd_t::STREAM_MODE_START_CONTINUOUS
                                        : uhd::stream_cmd_t::STREAM_MODE_NUM_SAMPS_AND_DONE);
        stream_cmd.num_samps  = size_t(num_requested_samples);
        stream_cmd.stream_now = rx_stream->get_num_channels() == 1;
        stream_cmd.time_spec  = usrp->get_time_now() + uhd::time_spec_t(0.05);
        rx_stream->issue_stream_cmd(stream_cmd);

        typedef std::map<size_t, size_t> SizeMap;
        SizeMap mapSizes;
        const auto start_time = std::chrono::steady_clock::now();
        const auto stop_time  = start_time + (1s * time_requested);
        // Track time and samps between updating the BW summary
        auto last_update                     = start_time;
        unsigned long long last_update_samps = 0;

        // Run this loop until either time expired (if a duration was given), until
        // the requested number of samples were collected (if such a number was
        // given), or until Ctrl-C was pressed.
        while (not stop_signal_called
            and (num_requested_samples != num_total_samps or num_requested_samples == 0)
            and (time_requested == 0.0 or std::chrono::steady_clock::now() <= stop_time)) {
            const auto now = std::chrono::steady_clock::now();

            size_t num_rx_samps =
                rx_stream->recv(buffs, samps_per_buff, md, 3.0, enable_size_map);

            if (md.error_code == uhd::rx_metadata_t::ERROR_CODE_TIMEOUT) {
                std::cout << std::endl
                        << thread_prefix << "Timeout while streaming" << std::endl;
                break;
            }
            if (md.error_code == uhd::rx_metadata_t::ERROR_CODE_OVERFLOW) {
                const std::lock_guard<std::mutex> lock(*recv_mutex);
                bool overflow_message   = true;
                if (overflow_message) {
                    overflow_message = false;
                    std::cerr
                        << boost::format(
                            "Got an overflow indication. Please consider the following:\n"
                            "  Your write medium must sustain a rate of %0.3fMB/s.\n"
                            "  Dropped samples will not be written to the file.\n"
                            "  Please modify this example for your purposes.\n"
                            "  This message will not appear again.\n")
                            % (usrp->get_rx_rate(channel_nums[0]) * total_num_channels
                                * sizeof(double) / 1e6);
                }
                continue;
            }
            if (md.error_code != uhd::rx_metadata_t::ERROR_CODE_NONE) {
                const std::lock_guard<std::mutex> lock(*recv_mutex);
                std::string error = thread_prefix + "Receiver error: " + md.strerror();
                if (continue_on_bad_packet) {
                    std::cerr << error << std::endl;
                    continue;
                } else
                    throw std::runtime_error(error);
            }

            if (enable_size_map) {
                const std::lock_guard<std::mutex> lock(*recv_mutex);
                SizeMap::iterator it = mapSizes.find(num_rx_samps);
                if (it == mapSizes.end())
                    mapSizes[num_rx_samps] = 0;
                mapSizes[num_rx_samps] += 1;
            }

            num_total_samps += num_rx_samps;

            for (size_t ch = 0; ch < rx_stream->get_num_channels(); ch++) {
                if (outfiles[ch].is_open()) {
                    outfiles[ch].write(
                        (const char*)buffs[ch], num_rx_samps * sizeof(double));
                }
            }

            last_update_samps += num_rx_samps;
            const auto time_since_last_update = now - last_update;
            if (time_since_last_update > 1s) {
                const std::lock_guard<std::mutex> lock(*recv_mutex);
                const double time_since_last_update_s =
                    std::chrono::duration<double>(time_since_last_update).count();
                bw                = double(last_update_samps) / time_since_last_update_s;
                last_update_samps = 0;
                last_update       = now;
            }
        }
        const auto actual_stop_time = std::chrono::steady_clock::now();

        stream_cmd.stream_mode = uhd::stream_cmd_t::STREAM_MODE_STOP_CONTINUOUS;
        rx_stream->issue_stream_cmd(stream_cmd);

        for (size_t i = 0; i < outfiles.size(); i++) {
            if (outfiles[i].is_open()) {
                outfiles[i].close();
            }
        }

        for (size_t i = 0; i < rx_stream->get_num_channels(); i++) {
            delete[] buffs[i];
        }


        if (stats) {
            const std::lock_guard<std::mutex> lock(*recv_mutex);
            std::cout << std::endl;
            const double actual_duration_seconds =
                std::chrono::duration<float>(actual_stop_time - start_time).count();
            std::cout << boost::format("%sReceived %d samples in %f seconds") % thread_prefix
                            % num_total_samps % actual_duration_seconds
                    << std::endl;

            if (enable_size_map) {
                std::cout << std::endl;
                std::cout << "Packet size map (bytes: count)" << std::endl;
                for (SizeMap::iterator it = mapSizes.begin(); it != mapSizes.end(); it++)
                    std::cout << it->first << ":\t" << it->second << std::endl;
            }
        }
    } // recv_samples_to_file



} //namespace HARDWARE