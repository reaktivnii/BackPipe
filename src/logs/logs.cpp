#include "logs.h"
#include <chrono>
#include <iostream>
#include <atomic>
#include <sstream>
#include <iomanip>

extern std::atomic<bool> isService;

std::string currentTime() {
    const auto now = std::chrono::system_clock::now();
    const std::time_t t_c = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&t_c), "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

void logInfo(const std::string& msg) {
    if (isService) std::cout << "[" << currentTime() << "] [INFO] " << msg << std::endl;
    else std::cout << "[INFO] " << msg << std::endl;
}

void logDebug(const std::string& msg) {
    if (isService) std::cout << "[" << currentTime() << "] [DEBUG] " << msg << std::endl;
    else std::cout << "[DEBUG] " << msg << std::endl;
}

void logError(const std::string& msg) {
    if (isService) std::cerr << "[" << currentTime() << "] [ERROR] " << msg << std::endl;
    else std::cerr << "[ERROR] " << msg << std::endl;
}

