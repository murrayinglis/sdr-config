#ifndef SFCW_HPP
#define SFCW_HPP

#include <iostream>
#include <vector>
#include <complex>
#include <queue>
#include <mutex>
#include <condition_variable>

#define IQ_3D std::vector<std::vector<std::vector<std::complex<double>>>>
#define IQ_2D std::vector<std::vector<std::complex<double>>>
#define IQ_1D std::vector<std::complex<double>>
namespace DSP{
    namespace SFCW{

        /// @brief Convenience enum for demodulation types in SFCW
        enum DEMODULATION_TYPE{
            HOMODYNE=0,
            HETERODYNE=1,
        };

        /// @brief Struct for the details of an SFCW sweep
        struct SFCW_SWEEP{
            int sweepID;
            double sweepStartTime_s; //since program 0 time
            std::vector<double> stepTimes_s; // since program 0 time
            std::string location;
            IQ_2D IQ_Mat;
            IQ_1D IQ_profile; //! Can probably delete later on. Useful for now
            IQ_1D rangeProfile;
            
            //default constructor
            SFCW_SWEEP(){}

            // constructor with just matrix
            SFCW_SWEEP(int id, double startTime, const IQ_2D& mat): sweepID(id), sweepStartTime_s(startTime), IQ_Mat(mat) {}

            //constructor with timeSpecs
            SFCW_SWEEP(int id, double startTime, std::vector<double> stepTimes, const IQ_2D& mat):sweepID(id), sweepStartTime_s(startTime),stepTimes_s(stepTimes), IQ_Mat(mat){}

        };

        /// @brief Thread safe implementation for a queue of SFCW structs for processing and storing 
        class SFCW_Sweep_Queue{
            public:
                SFCW_Sweep_Queue()= default;

                void push(const SFCW_SWEEP& sweepStruct) {
                    std::lock_guard<std::mutex> lock(m_mutex);
                    sweep_queue.push(sweepStruct);
                    m_condVar.notify_one();
                }

                bool pop(SFCW_SWEEP& sweepStruct) {
                    std::unique_lock<std::mutex> lock(m_mutex);
                    m_condVar.wait(lock, [this]{ return !sweep_queue.empty(); });
                    sweepStruct = sweep_queue.front();
                    sweep_queue.pop();
                    return true;
                }

                bool empty() const {
                    std::lock_guard<std::mutex> lock(m_mutex);
                    return sweep_queue.empty();
                }

                size_t size() const {
                    std::lock_guard<std::mutex> lock(m_mutex);
                    return sweep_queue.size();
                }


            private:
                std::queue<SFCW_SWEEP> sweep_queue;
                mutable std::mutex m_mutex;
                std::condition_variable m_condVar;
        };

        /// @brief uses values in the config file to set the SFCW parameters in CONFIG::waveform
        /// @param pMessageBuffer pointer to buffer for where details on how configuration is/is not achieved
        /// @return true if waveform can be configured false if not
        bool setSFCWParameters(std::string* pMessageBuffer);

        /// @brief Utility function for generating the times at which each step in a sweep should be Tx/Rx-ed
        /// @param startTime_s 
        /// @param stepLength_s 
        /// @param tuneTime_s 
        /// @param numSteps 
        /// @return 
        std::vector<double> getStepTimes(double startTime_s, double stepLength_s, double tuneTime_s, int numSteps);


        /// @brief Function to process the result of a sweep into a IQ vector. Will look into config to determine if homodyne or heterodyne
        /// @param sweepMat 2D IQ matrix resulting from an SFCW sweep
        /// @return 
        IQ_1D processSFCWSweepMat(IQ_2D sweepMat);

        /// @brief Consumer function for processing and storing a sueue of SFCW_Sweep objects. Will continue waiting for packets will pTestComplete=false
        /// @param sweepQueue 
        /// @param pTestComplete 
        void processQueueofSweeps(SFCW_Sweep_Queue& sweepQueue, bool* pTestComplete);
    }// namespace SFCW
}



#endif