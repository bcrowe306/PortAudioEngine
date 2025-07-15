#include "core/Logger.h"
#include <thread>
#include <chrono>

int main() {
    // Initialize the logger
    Logger::initialize();
    
    // Test different log levels
    Logger::trace("This is a trace message - very detailed debugging info");
    Logger::debug("This is a debug message - detailed debugging info");
    Logger::info("This is an info message - general information");
    Logger::warn("This is a warning message - something might be wrong");
    Logger::error("This is an error message - something went wrong");
    
    // Test formatted logging
    int value = 42;
    double pi = 3.14159;
    std::string name = "AudioEngine";
    
    Logger::info("Formatted logging test: value={}, pi={:.3f}, name={}", value, pi, name);
    
    // Test different log levels
    Logger::info("Testing different log levels:");
    
    Logger::setLevel(spdlog::level::trace);
    Logger::trace("TRACE level active - you should see this");
    
    Logger::setLevel(spdlog::level::info);
    Logger::trace("TRACE level disabled - you should NOT see this");
    Logger::info("INFO level active - you should see this");
    
    Logger::setLevel(spdlog::level::warn);
    Logger::info("INFO level disabled - you should NOT see this");
    Logger::warn("WARN level active - you should see this");
    
    // Performance test
    Logger::setLevel(spdlog::level::debug);
    Logger::info("Performance test - logging 1000 messages...");
    
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 1000; ++i) {
        Logger::debug("Debug message {}: value={}", i, i * 2);
    }
    auto end = std::chrono::high_resolution_clock::now();
    
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    Logger::info("Logged 1000 messages in {} microseconds", duration.count());
    
    Logger::info("Logger test completed successfully!");
    
    return 0;
}
