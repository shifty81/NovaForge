// Tests for: CaptainRelationshipSystem Tests
#include "test_log.h"
#include "components/fleet_components.h"
#include "ecs/system.h"
#include "systems/captain_relationship_system.h"

using namespace atlas;

// ==================== CaptainRelationshipSystem Tests ====================

static void testCaptainRelationshipRecordEvent() {
    std::cout << "\n=== Captain Relationship Record Event ===" << std::endl;
    ecs::World world;
    systems::CaptainRelationshipSystem sys(&world);
    world.createEntity("cap1");
    world.createEntity("cap2");
    sys.recordEvent("cap1", "cap2", "saved_in_combat");
    assertTrue(sys.getAffinity("cap1", "cap2") > 0.0f,
               "Affinity positive after saved_in_combat");
}

static void testCaptainRelationshipAbandoned() {
    std::cout << "\n=== Captain Relationship Abandoned ===" << std::endl;
    ecs::World world;
    systems::CaptainRelationshipSystem sys(&world);
    world.createEntity("cap1");
    world.createEntity("cap2");
    sys.recordEvent("cap1", "cap2", "abandoned");
    assertTrue(sys.getAffinity("cap1", "cap2") < 0.0f,
               "Affinity negative after abandoned");
}

static void testCaptainRelationshipStatus() {
    std::cout << "\n=== Captain Relationship Status Friend ===" << std::endl;
    ecs::World world;
    systems::CaptainRelationshipSystem sys(&world);
    world.createEntity("cap1");
    world.createEntity("cap2");
    // saved_in_combat gives +10 each, need >50
    for (int i = 0; i < 6; i++) {
        sys.recordEvent("cap1", "cap2", "saved_in_combat");
    }
    assertTrue(sys.getRelationshipStatus("cap1", "cap2") == "Friend",
               "Status is Friend with high affinity");
}

static void testCaptainRelationshipGrudge() {
    std::cout << "\n=== Captain Relationship Grudge ===" << std::endl;
    ecs::World world;
    systems::CaptainRelationshipSystem sys(&world);
    world.createEntity("cap1");
    world.createEntity("cap2");
    // abandoned gives -20 each, need < -50
    for (int i = 0; i < 3; i++) {
        sys.recordEvent("cap1", "cap2", "abandoned");
    }
    assertTrue(sys.getRelationshipStatus("cap1", "cap2") == "Grudge",
               "Status is Grudge with very negative affinity");
}

static void testCaptainRelationshipMultipleEvents() {
    std::cout << "\n=== Captain Relationship Multiple Events ===" << std::endl;
    ecs::World world;
    systems::CaptainRelationshipSystem sys(&world);
    world.createEntity("cap1");
    world.createEntity("cap2");
    sys.recordEvent("cap1", "cap2", "saved_in_combat");  // +10
    sys.recordEvent("cap1", "cap2", "abandoned");         // -20
    sys.recordEvent("cap1", "cap2", "shared_victory");    // +5
    // Net: -5
    float affinity = sys.getAffinity("cap1", "cap2");
    assertTrue(approxEqual(affinity, -5.0f), "Net affinity reflects mixed events");
}


void run_captain_relationship_system_tests() {
    testCaptainRelationshipRecordEvent();
    testCaptainRelationshipAbandoned();
    testCaptainRelationshipStatus();
    testCaptainRelationshipGrudge();
    testCaptainRelationshipMultipleEvents();
}
