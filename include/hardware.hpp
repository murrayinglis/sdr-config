#ifndef HARDWARE_HPP
#define HARDWARE_HPP
#include <uhd/usrp/multi_usrp.hpp>
#include <atomic>
#include <thread>

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

    void recv_to_file_doubles2(uhd::usrp::multi_usrp::sptr usrp,
        const std::string& file,
        int num_requested_samples,
        double settling_time,
        bool storeMD);

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
     * TX FUNCS
     * ----------------------------------------------
     */

    /*
    * TODO: templating for types
    */
    void transmitDoublesAtTime(uhd::usrp::multi_usrp::sptr tx_usrp, 
        std::vector<std::complex<double>> buffers, 
        double secondsInFuture, 
        uhd::tx_streamer::sptr tx_stream, 
        uhd::tx_metadata_t md);

    extern std::atomic_bool tx_stop_flag;
    void transmitDoublesUntilStopped(uhd::usrp::multi_usrp::sptr tx_usrp, 
        std::vector<std::complex<double>> buffers, 
        double secondsInFuture, 
        uhd::tx_streamer::sptr tx_stream, 
        uhd::tx_metadata_t md);


    /**
     * TX/RX FUNCS
     * 
     */
    extern std::atomic_bool sync_flag;

    void tx_doublesAtTimeSpec(uhd::usrp::multi_usrp::sptr usrp, 
        std::vector<std::complex<double>> buffers, 
        double secondsInFuture, 
        uhd::tx_streamer::sptr tx_stream, 
        uhd::tx_metadata_t md);

    void rx_doublesAtTimeAlert(uhd::usrp::multi_usrp::sptr usrp, 
        int num_requested_samples, 
        double rx_start_time, 
        uhd::rx_streamer::sptr rx_stream);
    
}

#endif