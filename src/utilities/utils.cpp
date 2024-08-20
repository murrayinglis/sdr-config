#include <iostream>
#include <uhd/usrp/multi_usrp.hpp>
#include "tests.hpp"
#include "config.hpp"
#include "utils.hpp"
#include <boost/format.hpp>
#include <regex>
#include <thread>
#include <fstream>
#include <filesystem>
#include <vector>
#include <cmath>
#include <boost/filesystem.hpp>
#include <chrono>

namespace utils{
    std::vector<std::complex<double>> read_in_complex_bin(std::string fileName)
    {
        std::vector<std::complex<double>> buffer;

        return buffer;
    }
    std::vector<std::complex<double>> read_in_complex_csv(std::string fileName)
    {
        std::vector<std::complex<double>> buffer;
        std::ifstream file(fileName);

        if (!file.is_open())
        {
            std::cerr << "Error: Could not open file " << fileName << std::endl;
            return buffer;
        }


        std::string line;
        // Check if the first line might be a header
        if (std::getline(file, line))
        {
            std::stringstream ss(line);
            std::string firstCell;
            std::getline(ss, firstCell, ',');

            // If the first cell contains non-numeric characters, it's likely a header
            if (std::any_of(firstCell.begin(), firstCell.end(), [](unsigned char c) { return !std::isdigit(c) && c != '.' && c != '-'; }))
            {
                //std::cout << "Detected header: " << line << std::endl;
            }
            else
            {
                // If the first line isn't a header, process it as data
                ss.clear();
                ss.str(line);
                std::string imagStr;

                if (std::getline(ss, firstCell, ',') && std::getline(ss, imagStr))
                {
                    try
                    {
                        double real = std::stod(firstCell);
                        double imag = std::stod(imagStr);
                        buffer.emplace_back(real, imag);
                    }
                    catch (const std::invalid_argument& e)
                    {
                        std::cerr << "Invalid number format: " << line << std::endl;
                    }
                    catch (const std::out_of_range& e)
                    {
                        std::cerr << "Number out of range: " << line << std::endl;
                    }
                }
            }
        }
    
            
        while (std::getline(file, line))
        {
            std::stringstream ss(line);
            std::string realStr, imagStr;

            if (std::getline(ss, realStr, ',') && std::getline(ss, imagStr))
            {
                double real = std::stod(realStr);
                double imag = std::stod(imagStr);
                buffer.emplace_back(real, imag);
            }
        }

        file.close();
        return buffer;
    }

    void print_all_params(std::string addr)
    {
        uhd::usrp::multi_usrp::sptr usrp = uhd::usrp::multi_usrp::make(std::string("addr=") += addr);

        // -------------------------------------------------------------------
        // RX PARAMS
        // subdevice

        // dc offset range
        uhd::meta_range_t rx_dc_offset_range = usrp->get_rx_dc_offset_range();
        std::cout << "RX DC offset range: " << rx_dc_offset_range.to_pp_string() << std::endl;


        // filters
        std::vector<std::string> rx_filters = usrp->get_rx_filter_names(0);
        std::cout << "RX Filters: " << std::endl;
        utils::ppVector(rx_filters);


        // antenna
        // TX/RX - can be set to either tx or rx
        // RX2 - set to receive
        // CAL - ?
        std::vector<std::string> rx_antennas = usrp->get_rx_antennas(0);
        std::cout << "RX Antennas: " << std::endl;
        utils::ppVector(rx_antennas);
        std::cout << std::endl;

        // RX rate range
        uhd::meta_range_t rx_rates = usrp->get_rx_rates(0);
        std::cout << "RX Rate range: " << rx_rates.start() << " to " << rx_rates.stop() << std::endl;
        std::cout << std::endl;


        // bandwidth range
        uhd::meta_range_t rx_bandwidth_range = usrp->get_rx_bandwidth_range(0);
        std::cout << "RX Bandwidth range: " << rx_bandwidth_range.to_pp_string() << std::endl;


        // freq range
        uhd::freq_range_t rx_freq_range = usrp->get_rx_freq_range(0);
        std::cout << "RX Freq range: " << rx_freq_range.to_pp_string() << std::endl;


        
        // gain names
        std::vector<std::string> rx_gain_names = usrp->get_rx_gain_names(0);
        std::cout << "RX gain names: " << std::endl;
        utils::ppVector(rx_gain_names);
        std::cout << std::endl;



        // gain profile names
        std::vector<std::string> rx_gain_profile_names = usrp->get_rx_gain_profile_names(0);
        std::cout << "RX gain profile names: " << std::endl;
        utils::ppVector(rx_gain_profile_names);



        // gain range
        uhd::gain_range_t rx_gain_range = usrp->get_rx_gain_range(0);
        std::cout << "RX gain range: " << rx_gain_range.to_pp_string() << std::endl;


        // power range
        try
        {
            uhd::meta_range_t rx_power_range = usrp->get_rx_power_range(0);
            std::cout << "RX power range:" << std::endl;
            std::cout << rx_power_range.start() << std::endl;
            std::cout << rx_power_range.stop() << std::endl;            
        }
        catch(const std::exception& e)
        {
            std::cerr << e.what() << '\n';
        }


        std::cout << std::endl;


        // LO freq range
        // LO names
        // LO sources
        // num channels

        // sensors

        // -------------------------------------------------------------------
        // TX PARAMS
        // subdevice

        // dc offset range
        uhd::meta_range_t tx_dc_offset_range = usrp->get_tx_dc_offset_range();
        std::cout << "TX DC offset range: " << tx_dc_offset_range.to_pp_string() << std::endl;
        std::cout << std::endl;


        // filters
        std::vector<std::string> tx_filters = usrp->get_tx_filter_names(0);
        std::cout << "TX Filters: " << std::endl;
        utils::ppVector(tx_filters);


        // antenna
        // RX/TX - can be set to either tx or rx
        std::vector<std::string> tx_antennas = usrp->get_tx_antennas(0);
        std::cout << "TX Antennas: " << std::endl;
        utils::ppVector(tx_antennas);
        std::cout << std::endl;

        // TX rate range
        uhd::meta_range_t tx_rates = usrp->get_tx_rates(0);
        std::cout << "TX Rate range: " << tx_rates.start() << " to " << tx_rates.stop() << std::endl;
        std::cout << std::endl;


        // bandwidth range
        uhd::meta_range_t tx_bandwidth_range = usrp->get_tx_bandwidth_range(0);
        std::cout << "TX Bandwidth range: " << tx_bandwidth_range.to_pp_string() << std::endl;


        // freq range
        uhd::freq_range_t tx_freq_range = usrp->get_tx_freq_range(0);
        std::cout << "TX Bandwidth range: " << tx_freq_range.to_pp_string() << std::endl;


        
        // gain names
        std::vector<std::string> tx_gain_names = usrp->get_tx_gain_names(0);
        std::cout << "TX gain names: " << std::endl;
        utils::ppVector(tx_gain_names);



        // gain profile names
        std::vector<std::string> tx_gain_profile_names = usrp->get_tx_gain_profile_names(0);
        std::cout << "TX gain profile names: " << std::endl;
        utils::ppVector(tx_gain_profile_names);



        // gain range
        uhd::gain_range_t tx_gain_range = usrp->get_tx_gain_range(0);
        std::cout << "TX gain range: " << tx_gain_range.to_pp_string() << std::endl;


        // power range
        try
        {
            uhd::meta_range_t tx_power_range = usrp->get_tx_power_range(0);
            std::cout << "TX power range:" << std::endl;
            std::cout << tx_power_range.start() << std::endl;
            std::cout << tx_power_range.stop() << std::endl;            
        }
        catch(const std::exception& e)
        {
            std::cerr << e.what() << '\n';
        }

    }


    void ppVector(std::vector<std::string> v)
    {
        for (size_t i = 0; i < v.size(); i++)
        {
            std::cout << v[i];
            if (i != v.size()-1)
            {
                std::cout << ", ";
            }
        }
        std::cout << std::endl;
    }
    
    bool fileAlreadyExists(std::string *pfilename, std::string extensionNoDot)
    {
        std::string fullpath=*pfilename+"."+extensionNoDot;
        if (std::filesystem::exists(fullpath)) {
            std::cout << "File already exists, renaming...\n";
            *pfilename=*pfilename+"_1";
            fileAlreadyExists(pfilename,extensionNoDot);
            return false;
        } else {
            return true;
        } 

        
        return false;
    }
    


    std::vector<std::complex<double>> generateComplexExp(double sampleRate, int signalLength, double frequency, double amplitude){
        std::vector<std::complex<double>> result;
        result.reserve(signalLength);

        
        if(amplitude>0.99||amplitude<0.1){
            std::cerr<<"generateComplexExp: amplitude too large or too small"<<std::endl;
            return result;
        }

        
        for (int i = 0; i < signalLength; ++i) {
            double t = static_cast<double>(i) / sampleRate;
            double phase = 2.0 * M_PI * frequency * t;
            result.push_back({amplitude*std::cos(phase),amplitude*std::cos(2*phase)});
        }

        return result;

    }

    std::vector<std::complex<double>> generateLinearSweep(double sampleRate, int signalLength, double startFrequency, double endFrequency){

        std::vector<std::complex<double>> result;
        result.reserve(signalLength);

        for (int i = 0; i < signalLength; ++i) {
            double t = static_cast<double>(i) / sampleRate;
            double frequency = startFrequency + (endFrequency - startFrequency) * i / signalLength;
            double phase = 2.0 * M_PI * frequency * t;
            std::complex<double> sample(0.5*std::cos(phase), 0);
            result.push_back(sample);
        }

        return result;
    }
    
    std::vector<double> linspace(double start, double end, int num) {
        std::vector<double> result;

        if (num <= 0) {
            return result;
        }

        if (num == 1) {
            result.push_back(start);
            return result;
        }

        double step = (end - start) / (num - 1);

        for (int i = 0; i < num; ++i) {
            result.push_back(start + i * step);
        }

        return result;
    }

    void printVecSize_t(std::vector<size_t> vec){
        for(int i=0;i<vec.size();i++){
            std::cout<<vec[i]<<", ";
        }
        std::cout<<std::endl;
        return;
    }

    void printIQMatrix(std::vector<std::vector<std::complex<double>>> mat){
        for (int row=0;row<mat.size();row++){
            for (int col=0;col<mat[0].size();col++){
                std::cout<<"{"<<mat[row][col].real()<<","<<mat[row][col].imag()<<"}, ";
            }
            std::cout<<"\n";
        }
    }

    uint64_t getCurrentEpochTime_ms(){
        uint64_t ms=std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
        return ms;
    }

    uint64_t getCurrentEpochTime_us(){
        uint64_t us=std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
        return us;
    }

    uint64_t getCurrentEpochTime_ns(){
        uint64_t ns=std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
        return ns;
    }

    uint64_t convertSecondsOffsetToEpoch(double offsetTime){
        uint64_t offsetMillis = static_cast<uint64_t>(std::round(offsetTime * 1000));
        return config::experimentZeroTime + offsetMillis;
    }

    std::vector<uint64_t> convertVecOffsetsToEpoch(std::vector<double> offsetsVec){
        std::vector<uint64_t> epochs;
        for (int i=0; i<offsetsVec.size();i++){
            epochs.push_back(convertSecondsOffsetToEpoch(offsetsVec[i]));
        }
        return epochs;
    }



}//namespace UTIL