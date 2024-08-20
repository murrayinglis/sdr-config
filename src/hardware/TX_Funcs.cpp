#include "hardware.hpp"
#include "utils.hpp"

#include <uhd/usrp/multi_usrp.hpp>
#include <fstream>

namespace hardware{

    void transmitDoublesAtTime(uhd::usrp::multi_usrp::sptr tx_usrp, 
        std::vector<std::complex<double>> buffers, 
        double secondsInFuture, 
        uhd::tx_streamer::sptr tx_stream, 
        uhd::tx_metadata_t& md)
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


        // metadata file (good idea Stephen)
        bool storeMD = true; // temp
        if (storeMD){
            std::ofstream metaDataFileStream;
            std::string metaDataFile="outputs/tx_metadata.txt";
            metaDataFileStream.open(metaDataFile);

            uint64_t currentEpoch_ns = utils::getCurrentEpochTime_ns();
            //std::cout<<currentEpoch_ns<<std::endl;

            auto timenow = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
	        metaDataFileStream << boost::format("System time at start: %s") % ctime(&timenow) << std::endl;
            metaDataFileStream << boost::format("Clock Reference: %s") % tx_usrp->get_clock_source(0) << std::endl;
            // TODO: finish this up


        }
        
        if(fullBufferLength<=maxTransmitSize){
            //std::cout << "Full buffer length <= max transmit size" << std::endl;
            std::vector<std::complex<double>*> pBuffs(1,&buffers.front());
            tx_stream->send(pBuffs,buffers.size(),md);
            md.end_of_burst=true;
            tx_stream->send("",0,md);
            return;
        }else{
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
                tx_stream->send(pBuffs,smallbuffer.size(),md);
                numSent+=smallBufferSize;
                
                md.has_time_spec=false; //dont want subsequent packets to wait
                md.start_of_burst=false;
            }

            // Send end of burst packet
            md.end_of_burst=true;
            tx_stream->send("",0,md);
            return;
        }

    }
} // namespace HARDWARE