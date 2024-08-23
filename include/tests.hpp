#ifndef TESTS_HPP
#define TESTS_HPP
#include <uhd/usrp/multi_usrp.hpp>
#include "config.hpp"

/**
 * @brief Contains the tests that can be performed.
 * @details This contains tests for simple RX/TX.
 * This contains radar tests for SFCW, FMCW, Pulse and Compressed Pulse.
 * @todo decide exactly which tests to include and implement them
 * 
 */
namespace tests{
    enum TEST_TYPE
    {
        HELLO_WORLD,
        GEN_SWEEP,
        CONFIG_TEST,

        RX_TEST,
        TX_TEST,

        TX_SINGLE_FREQ,
        TX_FROM_FILE,

        LOOPBACK_TEST,
        LOOPBACK_FROM_FILE,
        LATENCY,

        PULSED
    };

    namespace SFCW{
        
        /// @brief Runs a SFCW sweep based on the parameters given and returns a 2D matrix [stepsPerFreq,numFreqSteps] of IQ values 
        /// @param usrp 
        /// @param TransmitSignal This will be the signal transmitted for every step
        /// @param numsteps number of steps to use in the sweep
        /// @param stepSizeHz increment to be added to each step
        /// @param stepTimes vector of times at which each step should start
        /// @return 
        std::vector<std::vector<std::complex<double>>> performSweep(uhd::usrp::multi_usrp::sptr usrp, std::vector<std::complex<double>> TransmitSignal, int numsteps, double stepSizeHz, std::vector<double> stepTimes);

        /// @brief The main function for actually running an SFCW experiment. 
        /// @param usrp 
        /// @param numSweeps 
        /// @return 
        bool performNSweepsAndStore(uhd::usrp::multi_usrp::sptr usrp ,int numSweeps);

    }// namespace SFCW


    namespace LOOPBACK
    {
        void transmit_worker(uhd::usrp::multi_usrp::sptr tx_usrp, 
        std::vector<std::complex<double>> buffers, 
        double secondsInFuture, 
        uhd::tx_streamer::sptr tx_stream, 
        uhd::tx_metadata_t md);

        void transmit_worker_pulse(uhd::usrp::multi_usrp::sptr tx_usrp, 
        std::vector<std::complex<double>> buffers, 
        double secondsInFuture, 
        uhd::tx_streamer::sptr tx_stream, 
        uhd::tx_metadata_t md);
        
        void loopback_from_file(uhd::usrp::multi_usrp::sptr usrp, config::usrp_config usrp_config);
        
        void loopback(uhd::usrp::multi_usrp::sptr usrp, std::vector<std::complex<double>> buffers, double secondsInFuture, double settlingTime);

        void latency(uhd::usrp::multi_usrp::sptr usrp, std::vector<std::complex<double>> buffers, double secondsInFuture, double settlingTime);


        /// @brief Barebones test of downmixing a Oversampled IF frequency. Stores the following vectors: (complex)Received, downmixSig, singleChannel, mixedSignal 
        /// @param usrp 
        /// @param TransmitSignal 
        /// @param fileName 
        /// @param settling_time 
        void transmitReceiveDownmixToH5(uhd::usrp::multi_usrp::sptr usrp, std::vector<std::complex<double>> TransmitSignal, std::string fileName, double settling_time);

        void steppedFreqDumpToH5(uhd::usrp::multi_usrp::sptr usrp, std::vector<std::complex<double>> TransmitSignal, int numsteps, double stepSizeHz,std::string fileName);

        void steppedFreqOffsetTuningToH5(uhd::usrp::multi_usrp::sptr usrp, double basebandFreq, double offsetFreq, int numSamplesPerStep, double startingFreq, int numSteps, double stepSize, std::string fileNameNoExt);

        std::vector<std::vector<std::complex<double>>> steppedFreqToIQMatrix(uhd::usrp::multi_usrp::sptr usrp, std::vector<std::complex<double>> TransmitSignal, int numsteps, double initialTXFreq, double initialRXFreq ,double stepSizeHz);
    }

    namespace pulsed
    {
        void tx_rx_pulsed(uhd::usrp::multi_usrp::sptr usrp, size_t numSamples, std::string outputFile, std::string waveformFile);    
    }

    namespace RX
    {
        int rx_test(uhd::usrp::multi_usrp::sptr rx_usrp, size_t num_samples);
        int captureSingleFreqToFile(uhd::usrp::multi_usrp::sptr rx_usrp, std::string precision, size_t numSamples, std::string outputFile, double settling_time);
        
    }


    namespace TX
    {
        int tx_test(uhd::usrp::multi_usrp::sptr tx_usrp);

        int transmitSingleFreq(uhd::usrp::multi_usrp::sptr tx_usrp);
        int tx_waveform_from_file(uhd::usrp::multi_usrp::sptr tx_usrp, std::string precision, std::string waveformFilename, std::string outputFile, double secondsInFuture);
        void timeFreqSwitch(uhd::usrp::multi_usrp::sptr tx_usrp,int signalLength, int freqIncHz, int numIncrements);
    }

    namespace misc
    {
        void writeLinearSweepToFile(double sampleRate, int signalLength, double startFrequency, double endFrequency, const std::string& filename);
        int hello_world(std::string addr);
        int config_test(config::usrp_config usrp_config);
    }

    /**
     * @brief Call the test case specified in the config file.
     * 
     * @param usrp_config The config file
     */
    void handleTest(config::usrp_config usrp_config);

    /**
     * @brief List all the test types for printing purposes.
     * 
     * 
     */    
    void listTestTypes();

}// namespace tests


#endif