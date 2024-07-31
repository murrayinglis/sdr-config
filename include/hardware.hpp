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


    /**
     * RX CONFIG
     * ------------------------------------------------
     */
    bool setRXFreqHz(uhd::usrp::multi_usrp::sptr rx_usrp, double newRXFreqHz);

    bool confirmRxOscillatorsLocked(uhd::usrp::multi_usrp::sptr usrp_object, std::string ref_source,bool printing);

    int setupReceiever(uhd::usrp::multi_usrp::sptr tx_usrp);

    bool incrementRxFreq(uhd::usrp::multi_usrp::sptr rx_usrp, double incrementFreqHz);   


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
    void transmitDoublesAtTime(uhd::usrp::multi_usrp::sptr tx_usrp, std::vector<std::complex<double>> buffers, double secondsInFuture);

    void transmitShortsAtTime(uhd::usrp::multi_usrp::sptr tx_usrp, std::vector<std::complex<short>> buffers, double secondsInFuture);

    void endBurst(uhd::usrp::multi_usrp::sptr tx_usrp,uhd::tx_streamer::sptr tx_stream);    


    /**
     * TX CONFIG
     * ----------------------------------------------
     */
    bool confirmTxOscillatorsLocked(uhd::usrp::multi_usrp::sptr usrp_object, std::string ref_source, bool printing);

    int setupTransmitter(uhd::usrp::multi_usrp::sptr tx_usrp);

    bool setTxFreqHz(uhd::usrp::multi_usrp::sptr tx_usrp, double newTxFreqHz);

    bool incrementTxFreqHz(uhd::usrp::multi_usrp::sptr tx_usrp, double freqIncHz);    
}

#endif