// Tests for: ServerConsole Tests
#include "test_log.h"
#include "components/ui_components.h"
#include "utils/logger.h"
#include "ui/server_console.h"
#include <sys/stat.h>

using namespace atlas;

// ==================== ServerConsole Tests ====================

static void testConsoleInit() {
    std::cout << "\n=== Console Init ===" << std::endl;
    ServerConsole console;
    // Pass dummy references — the init only stores a flag
    bool ok = console.init();
    assertTrue(ok, "Console initializes successfully");
    assertTrue(console.getCommandCount() >= 2, "Built-in commands registered (help, status)");
}

static void testConsoleHelpCommand() {
    std::cout << "\n=== Console Help Command ===" << std::endl;
    ServerConsole console;
    console.init();

    std::string output = console.executeCommand("help");
    assertTrue(output.find("help") != std::string::npos, "Help output lists 'help' command");
    assertTrue(output.find("status") != std::string::npos, "Help output lists 'status' command");
}

static void testConsoleStatusCommand() {
    std::cout << "\n=== Console Status Command ===" << std::endl;
    ServerConsole console;
    console.init();

    std::string output = console.executeCommand("status");
    assertTrue(output.find("Server Status") != std::string::npos, "Status output has header");
    assertTrue(output.find("Commands registered") != std::string::npos, "Status shows command count");
}

static void testConsoleUnknownCommand() {
    std::cout << "\n=== Console Unknown Command ===" << std::endl;
    ServerConsole console;
    console.init();

    std::string output = console.executeCommand("foobar");
    assertTrue(output.find("Unknown command") != std::string::npos, "Unknown command error message");
}

static void testConsoleCustomCommand() {
    std::cout << "\n=== Console Custom Command ===" << std::endl;
    ServerConsole console;
    console.init();

    console.registerCommand("ping", "Reply with pong",
        [](const std::vector<std::string>& /*args*/) -> std::string {
            return "pong";
        });

    std::string output = console.executeCommand("ping");
    assertTrue(output == "pong", "Custom command returns expected output");
    assertTrue(console.getCommandCount() >= 3, "Custom command registered");
}

static void testConsoleLogBuffer() {
    std::cout << "\n=== Console Log Buffer ===" << std::endl;
    ServerConsole console;
    console.init();

    console.addLogMessage(utils::LogLevel::INFO, "Test message 1");
    console.addLogMessage(utils::LogLevel::INFO, "Test message 2");

    assertTrue(console.getLogBuffer().size() == 2, "Two log entries buffered");
    assertTrue(console.getLogBuffer()[0] == "Test message 1", "First log entry correct");
}

static void testConsoleEmptyCommand() {
    std::cout << "\n=== Console Empty Command ===" << std::endl;
    ServerConsole console;
    console.init();

    std::string output = console.executeCommand("");
    assertTrue(output.empty(), "Empty command returns empty string");
}

static void testConsoleNotInitialized() {
    std::cout << "\n=== Console Not Initialized ===" << std::endl;
    ServerConsole console;

    std::string output = console.executeCommand("help");
    assertTrue(output.find("not initialized") != std::string::npos, "Not-initialized message");
}

static void testConsoleShutdown() {
    std::cout << "\n=== Console Shutdown ===" << std::endl;
    ServerConsole console;
    console.init();
    assertTrue(console.getCommandCount() >= 2, "Commands before shutdown");

    console.shutdown();
    assertTrue(console.getCommandCount() == 0, "Commands cleared after shutdown");
}

static void testConsoleInteractiveMode() {
    std::cout << "\n=== Console Interactive Mode ===" << std::endl;
    ServerConsole console;
    assertTrue(!console.isInteractive(), "Default is non-interactive");
    console.setInteractive(true);
    assertTrue(console.isInteractive(), "Interactive mode set");
}


void run_server_console_tests() {
    testConsoleInit();
    testConsoleHelpCommand();
    testConsoleStatusCommand();
    testConsoleUnknownCommand();
    testConsoleCustomCommand();
    testConsoleLogBuffer();
    testConsoleEmptyCommand();
    testConsoleNotInitialized();
    testConsoleShutdown();
    testConsoleInteractiveMode();
}
