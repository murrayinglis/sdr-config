#include "tests.hpp"
#include <vector>
#include <complex>
#include <queue>
#include <atomic>
#include <uhd/usrp/multi_usrp.hpp>
#include "hardware.hpp"
#include "config.hpp"
#include <thread>

/**
 * TODO:
 * 
 * DylanTest branch has the hardcoded tests
 * 
 */
namespace tests{
    namespace SFCW{
        
        bool performNSweepsAndStore(uhd::usrp::multi_usrp::sptr usrp ,int numSweeps){
            bool testComplete = (false);

            return testComplete;

        }

        
        std::vector<std::vector<std::complex<double>>> performSweep(uhd::usrp::multi_usrp::sptr usrp, std::vector<std::complex<double>> TransmitSignal, int numsteps, double stepSizeHz, std::vector<double> stepTimes){
            
            std::vector<std::vector<std::complex<double>>> allSamples;

            return allSamples;
        }

    }// namespace SFCW
}//namespace tests