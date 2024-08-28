#include "MTimer.h"

MTimer::MTimer() : isRunning(false) {}

void MTimer::start() {
    startTime = std::chrono::high_resolution_clock::now();
    isRunning = true;
}

void MTimer::stop() {
    if (isRunning) {
        endTime = std::chrono::high_resolution_clock::now();
        isRunning = false;
    }
}

double MTimer::getDuration() const {
    std::chrono::duration<double> elapsed;
    if (isRunning) {
        elapsed = std::chrono::high_resolution_clock::now() - startTime;
    } else {
        elapsed = endTime - startTime;
    }
    return elapsed.count();
}
