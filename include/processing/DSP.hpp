#ifndef DSP_HPP
#define DSP_HPP

#include <iostream>
#include <vector>
#include <complex>
#include "processing/SFCW.hpp"

namespace DSP{

    /// @brief Return a vector of doubles representing either the I or Q component of the complex vector supplied
    /// @param signal IQ signal
    /// @param I_Or_Q int representing channel to be extracted. 0 for I, any other value for Q
    /// @return vector of doubles 
    std::vector<double> extractChannelFromIQ_d(std::vector<std::complex<double>> signal, int I_Or_Q);

    std::vector<std::vector<double>> extractChannelFromIQMat(const IQ_2D& IQ_mat, int I_or_Q);

    std::vector<double> generateCos_d(int num_samples, double sample_rate, double frequency, double phase_rad);

    std::vector<double> mixSignals_d(std::vector<double> sigA, std::vector<double> sigB);
    
    std::vector<std::vector<double>> digitalDemodulateToIQ(std::vector<double> signal, double downMixFreqHz, int sampleRateHz, double phaseShiftRad);

    std::complex<double> averageHomodyneFreq(std::vector<std::complex<double>> receivedSigIQ);

    std::complex<double> averageHeteroDyneFreq(std::vector<std::vector<double>> receivedSigIQ);

    std::vector<std::complex<double>> processHomodyneSweepMat(std::vector<std::vector<std::complex<double>>> IQMatrix);

    std::vector<std::complex<double>> processHeterodyneSweepMat(std::vector<std::vector<std::complex<double>>> IQMatrix);

    /// @brief Should replace processHeterodyneSweepMat() when done. 
    /// @param IQMatrix Matrix of [numSteps][samplesPerStep] of complex doubles that is the result of a SFCW sweep
    /// @param basebandFreq the frequency of the baseband signal
    /// @param offsetFreq the "SFCW_IF" or the offset tuned frequency
    /// @param mixOrFFT 1 for digitally downmix, 0 for FFT
    /// @return vector of complex doubles (len=numSteps) representing the frequency response of each step
    std::vector<std::complex<double>> processOffsetTunedSweepMat(std::vector<std::vector<std::complex<double>>> IQMatrix, double basebandFreq, double offsetFreq, int mixOrFFT);

}// namespace DSP


#endif