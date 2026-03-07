// Tests for: New Protocol Message Tests
#include "test_log.h"
#include "components/mission_components.h"
#include "systems/movement_system.h"
#include "network/protocol_handler.h"

using namespace atlas;

// ==================== New Protocol Message Tests ====================

static void testProtocolSalvageMessages() {
    std::cout << "\n=== Protocol: Salvage Messages ===" << std::endl;

    atlas::network::ProtocolHandler proto;

    // Parse salvage request
    std::string msg = "{\"message_type\":\"salvage_request\",\"data\":{\"wreck_id\":\"wreck_1\"}}";
    atlas::network::MessageType type;
    std::string data;
    bool ok = proto.parseMessage(msg, type, data);
    assertTrue(ok, "Salvage request parses successfully");
    assertTrue(type == atlas::network::MessageType::SALVAGE_REQUEST, "Type is SALVAGE_REQUEST");

    // Create salvage result
    std::string result = proto.createSalvageResult(true, "wreck_1", 3);
    assertTrue(result.find("salvage_result") != std::string::npos, "Result has correct type");
    assertTrue(result.find("wreck_1") != std::string::npos, "Result contains wreck_id");
    assertTrue(result.find("3") != std::string::npos, "Result contains items_recovered");
}

static void testProtocolLootMessages() {
    std::cout << "\n=== Protocol: Loot Messages ===" << std::endl;

    atlas::network::ProtocolHandler proto;

    std::string msg = "{\"message_type\":\"loot_all\",\"data\":{\"wreck_id\":\"wreck_2\"}}";
    atlas::network::MessageType type;
    std::string data;
    bool ok = proto.parseMessage(msg, type, data);
    assertTrue(ok, "Loot all parses successfully");
    assertTrue(type == atlas::network::MessageType::LOOT_ALL, "Type is LOOT_ALL");

    std::string result = proto.createLootResult(true, "wreck_2", 5, 10000.0);
    assertTrue(result.find("loot_result") != std::string::npos, "Result has correct type");
    assertTrue(result.find("wreck_2") != std::string::npos, "Result contains wreck_id");
    assertTrue(result.find("10000") != std::string::npos, "Result contains isc_gained");
}

static void testProtocolMiningMessages() {
    std::cout << "\n=== Protocol: Mining Messages ===" << std::endl;

    atlas::network::ProtocolHandler proto;

    // Parse mining start
    std::string msg = "{\"message_type\":\"mining_start\",\"data\":{\"deposit_id\":\"deposit_0\"}}";
    atlas::network::MessageType type;
    std::string data;
    bool ok = proto.parseMessage(msg, type, data);
    assertTrue(ok, "Mining start parses successfully");
    assertTrue(type == atlas::network::MessageType::MINING_START, "Type is MINING_START");

    // Parse mining stop
    msg = "{\"message_type\":\"mining_stop\",\"data\":{}}";
    ok = proto.parseMessage(msg, type, data);
    assertTrue(ok, "Mining stop parses successfully");
    assertTrue(type == atlas::network::MessageType::MINING_STOP, "Type is MINING_STOP");

    // Create mining result
    std::string result = proto.createMiningResult(true, "deposit_0", "Ferrite", 100);
    assertTrue(result.find("mining_result") != std::string::npos, "Result has correct type");
    assertTrue(result.find("Ferrite") != std::string::npos, "Result contains mineral_type");
    assertTrue(result.find("100") != std::string::npos, "Result contains quantity_mined");
}

static void testProtocolScannerMessages() {
    std::cout << "\n=== Protocol: Scanner Messages ===" << std::endl;

    atlas::network::ProtocolHandler proto;

    // Parse scan start
    std::string msg = "{\"message_type\":\"scan_start\",\"data\":{\"system_id\":\"sys_01\"}}";
    atlas::network::MessageType type;
    std::string data;
    bool ok = proto.parseMessage(msg, type, data);
    assertTrue(ok, "Scan start parses successfully");
    assertTrue(type == atlas::network::MessageType::SCAN_START, "Type is SCAN_START");

    // Parse scan stop
    msg = "{\"message_type\":\"scan_stop\",\"data\":{}}";
    ok = proto.parseMessage(msg, type, data);
    assertTrue(ok, "Scan stop parses successfully");
    assertTrue(type == atlas::network::MessageType::SCAN_STOP, "Type is SCAN_STOP");

    // Create scan result
    std::string result = proto.createScanResult("scanner_1", 2, "[{\"anomaly_id\":\"a1\"},{\"anomaly_id\":\"a2\"}]");
    assertTrue(result.find("scan_result") != std::string::npos, "Scan result has correct type");
    assertTrue(result.find("scanner_1") != std::string::npos, "Scan result contains scanner_id");
    assertTrue(result.find("\"anomalies_found\":2") != std::string::npos, "Scan result contains anomalies_found");
    assertTrue(result.find("a1") != std::string::npos, "Scan result contains first anomaly");
    assertTrue(result.find("a2") != std::string::npos, "Scan result contains second anomaly");
}

static void testProtocolAnomalyListMessages() {
    std::cout << "\n=== Protocol: Anomaly List Messages ===" << std::endl;

    atlas::network::ProtocolHandler proto;

    // Parse anomaly list request
    std::string msg = "{\"message_type\":\"anomaly_list\",\"data\":{\"system_id\":\"sys_02\"}}";
    atlas::network::MessageType type;
    std::string data;
    bool ok = proto.parseMessage(msg, type, data);
    assertTrue(ok, "Anomaly list request parses successfully");
    assertTrue(type == atlas::network::MessageType::ANOMALY_LIST, "Type is ANOMALY_LIST");

    // Create anomaly list response
    std::string result = proto.createAnomalyList("sys_02", 3, "[\"anom_1\",\"anom_2\",\"anom_3\"]");
    assertTrue(result.find("anomaly_list") != std::string::npos, "Anomaly list has correct type");
    assertTrue(result.find("sys_02") != std::string::npos, "Anomaly list contains system_id");
    assertTrue(result.find("\"count\":3") != std::string::npos, "Anomaly list contains count");
    assertTrue(result.find("anom_1") != std::string::npos, "Anomaly list contains first anomaly");
    assertTrue(result.find("anom_3") != std::string::npos, "Anomaly list contains third anomaly");
}

static void testProtocolScanResultParse() {
    std::cout << "\n=== Protocol: Scan Result Parse ===" << std::endl;

    atlas::network::ProtocolHandler proto;

    std::string msg = "{\"message_type\":\"scan_result\",\"data\":{\"scanner_id\":\"sc1\",\"anomalies_found\":1,\"results\":[{\"anomaly_id\":\"a1\",\"signal_strength\":0.75}]}}";
    atlas::network::MessageType type;
    std::string data;
    bool ok = proto.parseMessage(msg, type, data);
    assertTrue(ok, "Scan result parses successfully");
    assertTrue(type == atlas::network::MessageType::SCAN_RESULT, "Parsed type is SCAN_RESULT");
    assertTrue(data.find("sc1") != std::string::npos, "Data contains scanner_id");
}

static void testProtocolLootResultParse() {
    std::cout << "\n=== Protocol: Loot Result Parse ===" << std::endl;

    atlas::network::ProtocolHandler proto;

    std::string msg = "{\"message_type\":\"loot_result\",\"data\":{\"success\":true,\"wreck_id\":\"wreck_3\",\"items_collected\":2,\"isc_gained\":5000}}";
    atlas::network::MessageType type;
    std::string data;
    bool ok = proto.parseMessage(msg, type, data);
    assertTrue(ok, "Loot result parses successfully");
    assertTrue(type == atlas::network::MessageType::LOOT_RESULT, "Type is LOOT_RESULT");
}

static void testProtocolMiningResultParse() {
    std::cout << "\n=== Protocol: Mining Result Parse ===" << std::endl;

    atlas::network::ProtocolHandler proto;

    std::string msg = "{\"message_type\":\"mining_result\",\"data\":{\"success\":true,\"deposit_id\":\"deposit_1\",\"mineral_type\":\"Galvite\",\"quantity_mined\":50}}";
    atlas::network::MessageType type;
    std::string data;
    bool ok = proto.parseMessage(msg, type, data);
    assertTrue(ok, "Mining result parses successfully");
    assertTrue(type == atlas::network::MessageType::MINING_RESULT, "Type is MINING_RESULT");
}


void run_protocol_handler_tests() {
    testProtocolSalvageMessages();
    testProtocolLootMessages();
    testProtocolMiningMessages();
    testProtocolScannerMessages();
    testProtocolAnomalyListMessages();
    testProtocolScanResultParse();
    testProtocolLootResultParse();
    testProtocolMiningResultParse();
}
