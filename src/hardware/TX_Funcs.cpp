#include "hardware.hpp"
#include <uhd/usrp/multi_usrp.hpp>

namespace hardware{

    void transmitDoublesAtTime(uhd::usrp::multi_usrp::sptr tx_usrp, 
        std::vector<std::complex<double>> buffers, 
        double secondsInFuture, 
        uhd::tx_streamer::sptr tx_stream, 
        uhd::tx_metadata_t& md)
        {

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
            md.end_of_burst=true;
            tx_stream->send("",0,md);
            return;
        }

    }

    void transmitShortsAtTime(uhd::usrp::multi_usrp::sptr tx_usrp, std::vector<std::complex<short>> buffers, double secondsInFuture){
        //set up transmit streamer
        uhd::stream_args_t stream_args("fc32","sc16");
        uhd::tx_streamer::sptr tx_stream= tx_usrp->get_tx_stream(stream_args);
        
        uhd::tx_metadata_t md;
        md.has_time_spec=true;
        md.time_spec      =  uhd::time_spec_t(secondsInFuture);
        md.start_of_burst=true;

        size_t maxTransmitSize=tx_stream->get_max_num_samps(); //not entirely sure where this comes from
        //std::cout<<"Max Transmit Buffer Size: "<<maxTransmitSize<<"\n";
        size_t fullBufferLength=buffers.size();

        if(fullBufferLength<maxTransmitSize){
            std::vector<std::complex<short>*> pBuffs(1,&buffers.front());
            tx_stream->send(pBuffs,buffers.size(),md);
            md.end_of_burst=true;
            tx_stream->send("",0,md);
            return;
        }else{
            size_t numSent=0;
            while (numSent<fullBufferLength)
            {
                size_t smallBufferSize=fullBufferLength-numSent;
                if(smallBufferSize){
                    smallBufferSize=maxTransmitSize;
                }
                std::vector<std::complex<short>> smallbuffer(buffers.begin()+numSent,buffers.begin()+numSent+smallBufferSize);
                std::vector<std::complex<short>*> pBuffs(1,&smallbuffer.front());
                tx_stream->send(pBuffs,smallbuffer.size(),md);
                numSent+=smallBufferSize;
                
            }
            md.end_of_burst=true;
            tx_stream->send("",0,md);
            return;
        }   
    }
} // namespace HARDWARE