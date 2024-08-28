#ifndef MTIMER_H
#define MTIMER_H

#include <chrono>

class MTimer {
public:
    MTimer();                // Constructor
    void start();            // Start the timer
    void stop();             // Stop the timer
    double getDuration() const; // Get the duration in seconds

private:
    std::chrono::time_point<std::chrono::high_resolution_clock> startTime;
    std::chrono::time_point<std::chrono::high_resolution_clock> endTime;
    bool isRunning;
};

#endif // MTIMER_H
