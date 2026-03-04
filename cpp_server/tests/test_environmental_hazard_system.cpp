// Tests for: EnvironmentalHazardSystem Tests
#include "test_log.h"
#include "components/core_components.h"
#include "components/fps_components.h"
#include "ecs/component.h"
#include "ecs/system.h"
#include "systems/fps_character_controller_system.h"
#include "systems/movement_system.h"
#include "systems/environmental_hazard_system.h"

using namespace atlas;

// ==================== EnvironmentalHazardSystem Tests ====================

static void testEnvHazardCreate() {
    std::cout << "\n=== Environmental Hazard Create ===" << std::endl;
    ecs::World world;
    systems::EnvironmentalHazardSystem sys(&world);

    bool ok = sys.createHazard("hz_1", "room_eng", "interior_1",
                                components::EnvironmentalHazard::HazardType::Fire,
                                components::EnvironmentalHazard::Severity::Moderate);
    assertTrue(ok, "Create hazard succeeds");
    assertTrue(!sys.createHazard("hz_1", "room_eng", "interior_1",
                                  components::EnvironmentalHazard::HazardType::Fire,
                                  components::EnvironmentalHazard::Severity::Moderate),
               "Duplicate create fails");
}

static void testEnvHazardRoomSafety() {
    std::cout << "\n=== Environmental Hazard Room Safety ===" << std::endl;
    ecs::World world;
    systems::EnvironmentalHazardSystem sys(&world);

    assertTrue(sys.isRoomSafe("room_a"), "Empty room is safe");

    sys.createHazard("hz_1", "room_a", "interior_1",
                      components::EnvironmentalHazard::HazardType::HullBreach,
                      components::EnvironmentalHazard::Severity::Minor);
    assertTrue(!sys.isRoomSafe("room_a"), "Room with hazard is unsafe");
    assertTrue(sys.isRoomSafe("room_b"), "Other room still safe");
}

static void testEnvHazardDPS() {
    std::cout << "\n=== Environmental Hazard DPS ===" << std::endl;
    ecs::World world;
    systems::EnvironmentalHazardSystem sys(&world);

    sys.createHazard("hz_1", "room_a", "interior_1",
                      components::EnvironmentalHazard::HazardType::Fire,
                      components::EnvironmentalHazard::Severity::Minor);
    float dps = sys.getRoomDPS("room_a");
    assertTrue(dps > 0.0f, "Minor hazard has positive DPS");
    assertTrue(approxEqual(dps, 2.0f), "Minor hazard DPS is 2.0");
}

static void testEnvHazardDPSSeverity() {
    std::cout << "\n=== Environmental Hazard DPS Severity ===" << std::endl;
    ecs::World world;
    systems::EnvironmentalHazardSystem sys(&world);

    sys.createHazard("hz_cat", "room_a", "interior_1",
                      components::EnvironmentalHazard::HazardType::Fire,
                      components::EnvironmentalHazard::Severity::Catastrophic);
    float dps = sys.getRoomDPS("room_a");
    assertTrue(approxEqual(dps, 25.0f), "Catastrophic hazard DPS is 25.0");
}

static void testEnvHazardMultipleInRoom() {
    std::cout << "\n=== Environmental Hazard Multiple In Room ===" << std::endl;
    ecs::World world;
    systems::EnvironmentalHazardSystem sys(&world);

    sys.createHazard("hz_1", "room_a", "interior_1",
                      components::EnvironmentalHazard::HazardType::Fire,
                      components::EnvironmentalHazard::Severity::Minor);
    sys.createHazard("hz_2", "room_a", "interior_1",
                      components::EnvironmentalHazard::HazardType::Radiation,
                      components::EnvironmentalHazard::Severity::Moderate);

    auto hazards = sys.getHazardsInRoom("room_a");
    assertTrue(hazards.size() == 2, "Two hazards in room");
    float dps = sys.getRoomDPS("room_a");
    assertTrue(approxEqual(dps, 7.0f), "Combined DPS is 7.0 (2+5)");
}

static void testEnvHazardRepair() {
    std::cout << "\n=== Environmental Hazard Repair ===" << std::endl;
    ecs::World world;
    systems::EnvironmentalHazardSystem sys(&world);

    sys.createHazard("hz_1", "room_a", "interior_1",
                      components::EnvironmentalHazard::HazardType::Fire,
                      components::EnvironmentalHazard::Severity::Minor);
    assertTrue(sys.startRepair("hz_1"), "Start repair succeeds");
    assertTrue(approxEqual(sys.getRepairProgress("hz_1"), 0.0f), "Initial repair progress 0");

    // Simulate 10 seconds of repair (rate = 0.1/s → 1.0 progress)
    sys.update(10.0f);
    assertTrue(approxEqual(sys.getRepairProgress("hz_1"), 1.0f), "Repair complete after 10s");
    assertTrue(sys.isRoomSafe("room_a"), "Room safe after repair");
}

static void testEnvHazardStopRepair() {
    std::cout << "\n=== Environmental Hazard Stop Repair ===" << std::endl;
    ecs::World world;
    systems::EnvironmentalHazardSystem sys(&world);

    sys.createHazard("hz_1", "room_a", "interior_1",
                      components::EnvironmentalHazard::HazardType::Fire,
                      components::EnvironmentalHazard::Severity::Minor);
    sys.startRepair("hz_1");
    sys.update(3.0f);  // Partial repair
    sys.stopRepair("hz_1");
    float prog = sys.getRepairProgress("hz_1");
    assertTrue(prog > 0.0f && prog < 1.0f, "Partial repair progress");

    sys.update(5.0f);  // No further repair progress
    assertTrue(approxEqual(sys.getRepairProgress("hz_1"), prog), "No progress after stop");
}

static void testEnvHazardRemove() {
    std::cout << "\n=== Environmental Hazard Remove ===" << std::endl;
    ecs::World world;
    systems::EnvironmentalHazardSystem sys(&world);

    sys.createHazard("hz_1", "room_a", "interior_1",
                      components::EnvironmentalHazard::HazardType::Fire,
                      components::EnvironmentalHazard::Severity::Minor);
    assertTrue(!sys.isRoomSafe("room_a"), "Room unsafe with hazard");
    assertTrue(sys.removeHazard("hz_1"), "Remove hazard succeeds");
    assertTrue(sys.isRoomSafe("room_a"), "Room safe after removal");
}

static void testEnvHazardSpreadTimer() {
    std::cout << "\n=== Environmental Hazard Spread Timer ===" << std::endl;
    ecs::World world;
    systems::EnvironmentalHazardSystem sys(&world);

    sys.createHazard("hz_1", "room_a", "interior_1",
                      components::EnvironmentalHazard::HazardType::Fire,
                      components::EnvironmentalHazard::Severity::Catastrophic);

    auto* entity = world.getEntity("hz_1");
    auto* h = entity->getComponent<components::EnvironmentalHazard>();
    assertTrue(!h->is_spreading, "Not spreading initially");

    // Catastrophic spread interval = 5.0s
    sys.update(5.0f);
    assertTrue(h->is_spreading, "Spreading after interval");
}

static void testEnvHazardActiveList() {
    std::cout << "\n=== Environmental Hazard Active List ===" << std::endl;
    ecs::World world;
    systems::EnvironmentalHazardSystem sys(&world);

    sys.createHazard("hz_1", "room_a", "interior_1",
                      components::EnvironmentalHazard::HazardType::Fire,
                      components::EnvironmentalHazard::Severity::Minor);
    sys.createHazard("hz_2", "room_b", "interior_1",
                      components::EnvironmentalHazard::HazardType::Radiation,
                      components::EnvironmentalHazard::Severity::Moderate);
    sys.createHazard("hz_3", "room_c", "interior_2",
                      components::EnvironmentalHazard::HazardType::ToxicLeak,
                      components::EnvironmentalHazard::Severity::Critical);

    auto active1 = sys.getActiveHazards("interior_1");
    assertTrue(active1.size() == 2, "2 active hazards in interior_1");

    auto active2 = sys.getActiveHazards("interior_2");
    assertTrue(active2.size() == 1, "1 active hazard in interior_2");
}

static void testEnvHazardTypeName() {
    std::cout << "\n=== Environmental Hazard Type Names ===" << std::endl;
    assertTrue(systems::EnvironmentalHazardSystem::hazardTypeName(0) == "HullBreach", "HullBreach name");
    assertTrue(systems::EnvironmentalHazardSystem::hazardTypeName(1) == "Fire", "Fire name");
    assertTrue(systems::EnvironmentalHazardSystem::hazardTypeName(2) == "Radiation", "Radiation name");
    assertTrue(systems::EnvironmentalHazardSystem::severityName(0) == "Minor", "Minor severity");
    assertTrue(systems::EnvironmentalHazardSystem::severityName(3) == "Catastrophic", "Catastrophic severity");
}

static void testEnvHazardComponentDefaults() {
    std::cout << "\n=== Environmental Hazard Component Defaults ===" << std::endl;
    components::EnvironmentalHazard h;
    assertTrue(h.is_active == true, "Default active");
    assertTrue(!h.is_spreading, "Default not spreading");
    assertTrue(!h.is_being_repaired, "Default not being repaired");
    assertTrue(approxEqual(h.repair_progress, 0.0f), "Default repair 0");
    assertTrue(approxEqual(h.damage_per_second, 5.0f), "Default DPS 5.0");
    assertTrue(approxEqual(h.spread_interval, 30.0f), "Default spread interval 30");
}

static void testEnvHazardRoomLevelDamage() {
    std::cout << "\n=== Environmental Hazard Room-Level Damage ===" << std::endl;
    ecs::World world;
    systems::EnvironmentalHazardSystem hazSys(&world);
    systems::FPSCharacterControllerSystem fpsSys(&world);

    // Create a fire hazard in room_eng
    hazSys.createHazard("hz_fire", "room_eng", "interior_ship",
                         components::EnvironmentalHazard::HazardType::Fire,
                         components::EnvironmentalHazard::Severity::Minor);

    // Spawn two characters in the same interior
    fpsSys.spawnCharacter("player_in_room", "interior_ship", 0, 0, 0);
    fpsSys.spawnCharacter("player_other_room", "interior_ship", 0, 0, 0);

    // Set rooms: one in the hazard room, one elsewhere
    fpsSys.setCurrentRoom("player_in_room", "room_eng");
    fpsSys.setCurrentRoom("player_other_room", "room_bridge");

    // Give both characters health
    auto* e1 = world.getEntity("fpschar_player_in_room");
    auto h1 = std::make_unique<components::FPSHealth>();
    h1->health = 100.0f; h1->shield = 0.0f;
    e1->addComponent(std::move(h1));

    auto* e2 = world.getEntity("fpschar_player_other_room");
    auto h2 = std::make_unique<components::FPSHealth>();
    h2->health = 100.0f; h2->shield = 0.0f;
    e2->addComponent(std::move(h2));

    // Update for 1 second (Minor DPS = 2.0)
    hazSys.update(1.0f);

    auto* hp1 = e1->getComponent<components::FPSHealth>();
    auto* hp2 = e2->getComponent<components::FPSHealth>();
    assertTrue(hp1->health < 100.0f, "Player in hazard room takes damage");
    assertTrue(approxEqual(hp1->health, 98.0f), "Player takes 2.0 damage in 1s");
    assertTrue(approxEqual(hp2->health, 100.0f), "Player in other room takes no damage");
}

static void testEnvHazardNoRoomNoDamage() {
    std::cout << "\n=== Environmental Hazard No Room No Damage ===" << std::endl;
    ecs::World world;
    systems::EnvironmentalHazardSystem hazSys(&world);
    systems::FPSCharacterControllerSystem fpsSys(&world);

    hazSys.createHazard("hz_1", "room_eng", "interior_ship",
                         components::EnvironmentalHazard::HazardType::Fire,
                         components::EnvironmentalHazard::Severity::Moderate);

    // Spawn character with empty current_room_id (default)
    fpsSys.spawnCharacter("player_no_room", "interior_ship", 0, 0, 0);

    auto* entity = world.getEntity("fpschar_player_no_room");
    auto hp = std::make_unique<components::FPSHealth>();
    hp->health = 100.0f; hp->shield = 0.0f;
    entity->addComponent(std::move(hp));

    hazSys.update(1.0f);

    auto* health = entity->getComponent<components::FPSHealth>();
    assertTrue(approxEqual(health->health, 100.0f), "Character without room takes no damage");
}

static void testEnvHazardRoomDamageShieldCascade() {
    std::cout << "\n=== Environmental Hazard Room Damage Shield Cascade ===" << std::endl;
    ecs::World world;
    systems::EnvironmentalHazardSystem hazSys(&world);
    systems::FPSCharacterControllerSystem fpsSys(&world);

    hazSys.createHazard("hz_1", "room_eng", "interior_ship",
                         components::EnvironmentalHazard::HazardType::Radiation,
                         components::EnvironmentalHazard::Severity::Catastrophic);

    fpsSys.spawnCharacter("shielded", "interior_ship", 0, 0, 0);
    fpsSys.setCurrentRoom("shielded", "room_eng");

    auto* entity = world.getEntity("fpschar_shielded");
    auto hp = std::make_unique<components::FPSHealth>();
    hp->health = 100.0f; hp->shield = 10.0f;
    entity->addComponent(std::move(hp));

    // Catastrophic DPS = 25.0, 1 second → 25 damage
    // Shield absorbs 10, remaining 15 hits health
    hazSys.update(1.0f);

    auto* health = entity->getComponent<components::FPSHealth>();
    assertTrue(approxEqual(health->shield, 0.0f), "Shield depleted");
    assertTrue(approxEqual(health->health, 85.0f), "Health takes overflow damage");
}

static void testFPSCharControllerCurrentRoom() {
    std::cout << "\n=== FPS Character Controller Current Room ===" << std::endl;
    ecs::World world;
    systems::FPSCharacterControllerSystem sys(&world);

    sys.spawnCharacter("player1", "interior_ship", 0, 0, 0);
    assertTrue(sys.getCurrentRoom("player1").empty(), "Default room is empty");

    assertTrue(sys.setCurrentRoom("player1", "room_bridge"), "Set room succeeds");
    assertTrue(sys.getCurrentRoom("player1") == "room_bridge", "Room is bridge");

    assertTrue(sys.setCurrentRoom("player1", "room_eng"), "Change room succeeds");
    assertTrue(sys.getCurrentRoom("player1") == "room_eng", "Room is engineering");

    assertTrue(!sys.setCurrentRoom("nonexistent", "room_a"), "Set room fails for missing player");
    assertTrue(sys.getCurrentRoom("nonexistent").empty(), "Get room empty for missing player");
}


void run_environmental_hazard_system_tests() {
    testEnvHazardCreate();
    testEnvHazardRoomSafety();
    testEnvHazardDPS();
    testEnvHazardDPSSeverity();
    testEnvHazardMultipleInRoom();
    testEnvHazardRepair();
    testEnvHazardStopRepair();
    testEnvHazardRemove();
    testEnvHazardSpreadTimer();
    testEnvHazardActiveList();
    testEnvHazardTypeName();
    testEnvHazardComponentDefaults();
    testEnvHazardRoomLevelDamage();
    testEnvHazardNoRoomNoDamage();
    testEnvHazardRoomDamageShieldCascade();
    testFPSCharControllerCurrentRoom();
}
