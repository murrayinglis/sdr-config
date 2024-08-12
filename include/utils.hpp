#ifndef UTILITIES_HPP
#define UTILITIES_HPP

#include <iostream>
#include <uhd/usrp/multi_usrp.hpp>
#include "config.hpp"
#include <boost/format.hpp>
#include <regex>
#include <thread>
#include <boost/filesystem.hpp>
#include <filesystem>


/// @brief 
namespace utils{    
    void ppVector(std::vector<std::string> v);
    
    bool fileAlreadyExists(std::string *pfilename, std::string extensionNoDot);

    int readInWaveformFile(); // File type?

    std::vector<std::complex<double>> generateLinearSweep(double sampleRate, int signalLength, double startFrequency, double endFrequency);

    std::vector<std::complex<double>> generateComplexExp(double sampleRate, int signalLength, double frequency, double amplitude);

    std::vector<double> linspace(double start, double end, int num);

    void printVecSize_t(std::vector<size_t> vec);

    void printIQMatrix(std::vector<std::vector<std::complex<double>>> mat);

    uint64_t getCurrentEpochTime_ms();

    uint64_t getCurrentEpochTime_us();

    /// @brief Converts from a double (offset) used for timing USRP stream commands to an equivalent epoch time 
    /// @param offsetTime 
    /// @return offsetTime in millis + experiment_start_time
    uint64_t convertSecondsOffsetToEpoch(double offsetTime);

    /// @brief Converts a vector of offsets (doubles) to their equivalent epoch times
    /// @param offsetsVec 
    /// @return 
    std::vector<uint64_t> convertVecOffsetsToEpoch(std::vector<double> offsetsVec);

}//UTIL

#endif