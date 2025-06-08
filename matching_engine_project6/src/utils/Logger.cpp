
// ===== src/utils/Logger.cpp =====
#include "utils/Logger.hpp"

std::ofstream Logger::logFile_;
bool Logger::enabled_ = false;

void Logger::init(const std::string& filename) {
    if (logFile_.is_open()) {
        logFile_.close();
    }
    logFile_.open(filename, std::ios::app);
    enabled_ = true;
}

void Logger::log(const std::string& message) {
    if (!enabled_) return;
    
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    
    if (logFile_.is_open()) {
        logFile_ << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S")
                 << " - " << message << std::endl;
    }
}

void Logger::close() {
    if (logFile_.is_open()) {
        logFile_.close();
    }
}