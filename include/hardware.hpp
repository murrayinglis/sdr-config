#ifndef HARDWARE_HPP
#define HARDWARE_HPP
#include <uhd/usrp/multi_usrp.hpp>

namespace hardware{
    /**
     * RX FUNCS
     * -------------------------------------------------
     */
    std::vector<std::complex<double>> captureDoubles(uhd::usrp::multi_usrp::sptr rx_usrp,size_t numSamples,double settling_time);
    
    void recv_to_file_doubles(uhd::usrp::multi_usrp::sptr usrp,
        const std::string& file,
        int num_requested_samples,
        double settling_time,
        bool storeMD);

    // remove
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
    const std::string& thread_prefix);



    /**
     * TXRX CONFIG
     * -----------------------------------------------
     */
    void synhronousTXRXTune_basic(uhd::usrp::multi_usrp::sptr usrp, double txFreqHz, double rxFreqHz, double timeOffset);

    void synchronousTXRCTune_manual(uhd::usrp::multi_usrp::sptr usrp,double txFreqHz, double rxFreqHz, double timeOffset);

    void synchronousTXRXIncrement(uhd::usrp::multi_usrp::sptr usrp, double incrementAmountHz,double timeOffset);

    /// @brief Ensures LO locked for both TX and RX chains. 
    /// @param usrp 
    /// @param timeout upper bound on the time to wait for lock
    /// @return True if both managed to lock within the timeFrame
    bool checkTXRXLOsLocked(uhd::usrp::multi_usrp::sptr usrp, double timeout);    


    /**
     * TXRX FUNCS
     * ----------------------------------------------
     */
    void transmitDoublesAtTime(uhd::usrp::multi_usrp::sptr tx_usrp, 
        std::vector<std::complex<double>> buffers, 
        double secondsInFuture, 
        uhd::tx_streamer::sptr tx_stream, 
        uhd::tx_metadata_t& md);

    void transmitShortsAtTime(uhd::usrp::multi_usrp::sptr tx_usrp, std::vector<std::complex<short>> buffers, double secondsInFuture);

    void endBurst(uhd::usrp::multi_usrp::sptr tx_usrp,uhd::tx_streamer::sptr tx_stream);    



    /**
     * TX FUNCS
     * -----------------------------------------------
     */
    
    
}

#endif