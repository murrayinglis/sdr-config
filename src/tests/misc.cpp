#include "tests.hpp"
#include "utils.hpp"
#include <uhd/usrp/multi_usrp.hpp>
#include "config.hpp"
#include <iostream>
#include <fstream>
#include "hardware.hpp"

namespace tests{
    namespace misc {
        void writeLinearSweepToFile(double sampleRate, int signalLength, double startFrequency, double endFrequency, const std::string& filename)
        {
            std::vector<std::complex<double>> chirp = utils::generateLinearSweep(sampleRate, signalLength, startFrequency, endFrequency);

            std::ofstream file(filename);
            
            if (!file.is_open()) {
                std::cerr << "Error opening file: " << filename << std::endl;
                return;
            }

            // Write header (optional)
            file << "Real,Imaginary\n";
            
            // Write data
            for (const auto& value : chirp) {
                file << std::fixed << std::setprecision(6)  // Set precision for floating-point numbers
                    << value.real() << ','  // Write real part
                    << value.imag() << '\n'; // Write imaginary part
            }

            file.close();
        }
    }

} // namespace test_types