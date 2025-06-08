// ===== include/utils/Logger.hpp =====
#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include <iomanip>

class Logger {
private:
    static std::ofstream logFile_;
    static bool enabled_;
    
public:
    static void init(const std::string& filename);
    static void enable(bool enable) { enabled_ = enable; }
    static void log(const std::string& message);
    static void close();
};
