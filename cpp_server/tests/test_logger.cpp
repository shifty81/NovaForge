// Tests for: Logger Tests
#include "test_log.h"
#include "utils/logger.h"
#include <fstream>

using namespace atlas;

// ==================== Logger Tests ====================

static void testLoggerLevels() {
    std::cout << "\n=== Logger Levels ===" << std::endl;
    
    auto& log = utils::Logger::instance();
    
    // Disable console output so tests don't clutter the terminal
    log.setConsoleOutput(false);
    
    log.setLevel(utils::LogLevel::INFO);
    assertTrue(log.getLevel() == utils::LogLevel::INFO, "Log level set to INFO");
    
    log.setLevel(utils::LogLevel::DEBUG);
    assertTrue(log.getLevel() == utils::LogLevel::DEBUG, "Log level set to DEBUG");
    
    log.setLevel(utils::LogLevel::ERROR);
    assertTrue(log.getLevel() == utils::LogLevel::ERROR, "Log level set to ERROR");
    
    log.setLevel(utils::LogLevel::WARN);
    assertTrue(log.getLevel() == utils::LogLevel::WARN, "Log level set to WARN");
    
    log.setLevel(utils::LogLevel::FATAL);
    assertTrue(log.getLevel() == utils::LogLevel::FATAL, "Log level set to FATAL");
    
    // Re-enable console output
    log.setConsoleOutput(true);
    // Reset to INFO for other tests
    log.setLevel(utils::LogLevel::INFO);
}

static void testLoggerFileOutput() {
    std::cout << "\n=== Logger File Output ===" << std::endl;
    
    auto& log = utils::Logger::instance();
    log.setConsoleOutput(false);
    
    // Shut down any previously opened file
    log.shutdown();
    assertTrue(!log.isFileOpen(), "No file open after shutdown");
    
    // Init with a temp directory
    bool ok = log.init("/tmp/eve_test_logs");
    assertTrue(ok, "Logger init succeeds");
    assertTrue(log.isFileOpen(), "Log file is open after init");
    
    // Write some log entries
    log.setLevel(utils::LogLevel::DEBUG);
    log.debug("test debug message");
    log.info("test info message");
    log.warn("test warn message");
    log.error("test error message");
    
    log.shutdown();
    assertTrue(!log.isFileOpen(), "Log file closed after shutdown");
    
    // Verify the file was actually written
    std::ifstream f("/tmp/eve_test_logs/server.log");
    assertTrue(f.is_open(), "Log file exists on disk");
    
    std::string content((std::istreambuf_iterator<char>(f)),
                         std::istreambuf_iterator<char>());
    assertTrue(content.find("[DEBUG]") != std::string::npos, "Log contains DEBUG entry");
    assertTrue(content.find("[INFO]") != std::string::npos, "Log contains INFO entry");
    assertTrue(content.find("[WARN]") != std::string::npos, "Log contains WARN entry");
    assertTrue(content.find("[ERROR]") != std::string::npos, "Log contains ERROR entry");
    assertTrue(content.find("test debug message") != std::string::npos, "Log contains debug text");
    assertTrue(content.find("test info message") != std::string::npos, "Log contains info text");
    f.close();
    
    // Clean up
    std::remove("/tmp/eve_test_logs/server.log");
    
    // Re-enable console
    log.setConsoleOutput(true);
    log.setLevel(utils::LogLevel::INFO);
}

static void testLoggerLevelFiltering() {
    std::cout << "\n=== Logger Level Filtering ===" << std::endl;
    
    auto& log = utils::Logger::instance();
    log.setConsoleOutput(false);
    log.shutdown();
    
    bool ok = log.init("/tmp/eve_test_logs", "filter_test.log");
    assertTrue(ok, "Logger init for filter test succeeds");
    
    // Set level to WARN — DEBUG and INFO should be filtered out
    log.setLevel(utils::LogLevel::WARN);
    log.debug("should_not_appear_debug");
    log.info("should_not_appear_info");
    log.warn("should_appear_warn");
    log.error("should_appear_error");
    
    log.shutdown();
    
    std::ifstream f("/tmp/eve_test_logs/filter_test.log");
    assertTrue(f.is_open(), "Filter test log file exists");
    
    std::string content((std::istreambuf_iterator<char>(f)),
                         std::istreambuf_iterator<char>());
    assertTrue(content.find("should_not_appear_debug") == std::string::npos,
               "DEBUG filtered out at WARN level");
    assertTrue(content.find("should_not_appear_info") == std::string::npos,
               "INFO filtered out at WARN level");
    assertTrue(content.find("should_appear_warn") != std::string::npos,
               "WARN passes at WARN level");
    assertTrue(content.find("should_appear_error") != std::string::npos,
               "ERROR passes at WARN level");
    f.close();
    
    std::remove("/tmp/eve_test_logs/filter_test.log");
    log.setConsoleOutput(true);
    log.setLevel(utils::LogLevel::INFO);
}


void run_logger_tests() {
    testLoggerLevels();
    testLoggerFileOutput();
    testLoggerLevelFiltering();
}
