// Tests for: Star System Manager System Tests
#include "test_log.h"
#include "components/core_components.h"
#include "components/game_components.h"
#include "ecs/system.h"
#include "systems/star_system_manager_system.h"

using namespace atlas;

// ==================== Star System Manager System Tests ====================

static void testStarSystemManagerCreate() {
    std::cout << "\n=== StarSystemManager: Create ===" << std::endl;
    ecs::World world;
    systems::StarSystemManagerSystem sys(&world);
    world.createEntity("sys1");
    assertTrue(sys.initialize("sys1", "Jita", 1.0f), "Init succeeds");
    assertTrue(sys.getCelestialCount("sys1") == 0, "No celestials initially");
    assertTrue(sys.getStationCount("sys1") == 0, "No stations initially");
    assertTrue(sys.getGateCount("sys1") == 0, "No gates initially");
    assertTrue(sys.getNPCFactionCount("sys1") == 0, "No NPC factions initially");
    assertTrue(approxEqual(sys.getSecurityStatus("sys1"), 1.0f), "Security is 1.0");
    assertTrue(sys.getTotalDockings("sys1") == 0, "No dockings initially");
    assertTrue(sys.getTotalJumps("sys1") == 0, "No jumps initially");
}

static void testStarSystemManagerCelestials() {
    std::cout << "\n=== StarSystemManager: Celestials ===" << std::endl;
    ecs::World world;
    systems::StarSystemManagerSystem sys(&world);
    world.createEntity("sys1");
    sys.initialize("sys1", "Jita", 1.0f);
    assertTrue(sys.addCelestial("sys1", "star1", "Jita Star", "Star", 0, 0, 0, 50000), "Add star");
    assertTrue(sys.addCelestial("sys1", "p1", "Jita IV", "Planet", 10000, 0, 0, 5000), "Add planet");
    assertTrue(sys.addCelestial("sys1", "belt1", "Jita IV Belt", "AsteroidBelt", 12000, 0, 0, 1000), "Add belt");
    assertTrue(sys.getCelestialCount("sys1") == 3, "3 celestials");
    assertTrue(!sys.addCelestial("sys1", "star1", "Dup", "Star", 0, 0, 0, 100), "Duplicate rejected");
    assertTrue(sys.removeCelestial("sys1", "p1"), "Remove planet");
    assertTrue(sys.getCelestialCount("sys1") == 2, "2 celestials after remove");
    assertTrue(!sys.removeCelestial("sys1", "p1"), "Double remove fails");
}

static void testStarSystemManagerStations() {
    std::cout << "\n=== StarSystemManager: Stations ===" << std::endl;
    ecs::World world;
    systems::StarSystemManagerSystem sys(&world);
    world.createEntity("sys1");
    sys.initialize("sys1", "Jita", 1.0f);
    assertTrue(sys.addStation("sys1", "st1", "Jita 4-4", "Caldari", 10000, 500, 0), "Add station");
    assertTrue(sys.getStationCount("sys1") == 1, "1 station");
    assertTrue(!sys.addStation("sys1", "st1", "Dup", "Caldari", 0, 0, 0), "Duplicate rejected");
    assertTrue(sys.removeStation("sys1", "st1"), "Remove station");
    assertTrue(sys.getStationCount("sys1") == 0, "0 stations after remove");
}

static void testStarSystemManagerDocking() {
    std::cout << "\n=== StarSystemManager: Docking ===" << std::endl;
    ecs::World world;
    systems::StarSystemManagerSystem sys(&world);
    world.createEntity("sys1");
    sys.initialize("sys1", "Jita", 1.0f);
    sys.addStation("sys1", "st1", "Jita 4-4", "Caldari", 10000, 500, 0);
    assertTrue(sys.dockAtStation("sys1", "st1"), "Dock succeeds");
    assertTrue(sys.getStationDockedCount("sys1", "st1") == 1, "1 docked");
    assertTrue(sys.dockAtStation("sys1", "st1"), "Dock again succeeds");
    assertTrue(sys.getStationDockedCount("sys1", "st1") == 2, "2 docked");
    assertTrue(sys.getTotalDockings("sys1") == 2, "2 total dockings");
    assertTrue(sys.undockFromStation("sys1", "st1"), "Undock succeeds");
    assertTrue(sys.getStationDockedCount("sys1", "st1") == 1, "1 docked after undock");
}

static void testStarSystemManagerDockingFull() {
    std::cout << "\n=== StarSystemManager: DockingFull ===" << std::endl;
    ecs::World world;
    systems::StarSystemManagerSystem sys(&world);
    world.createEntity("sys1");
    sys.initialize("sys1", "Jita", 1.0f);
    sys.addStation("sys1", "st1", "Jita 4-4", "Caldari", 10000, 500, 0);
    auto* entity = world.getEntity("sys1");
    auto* state = entity->getComponent<components::StarSystemState>();
    state->stations[0].max_docking = 2;
    assertTrue(sys.dockAtStation("sys1", "st1"), "Dock 1");
    assertTrue(sys.dockAtStation("sys1", "st1"), "Dock 2");
    assertTrue(!sys.dockAtStation("sys1", "st1"), "Dock 3 fails (full)");
}

static void testStarSystemManagerGates() {
    std::cout << "\n=== StarSystemManager: Gates ===" << std::endl;
    ecs::World world;
    systems::StarSystemManagerSystem sys(&world);
    world.createEntity("sys1");
    sys.initialize("sys1", "Jita", 1.0f);
    assertTrue(sys.addGate("sys1", "g1", "Perimeter", 50000, 0, 0), "Add gate");
    assertTrue(sys.addGate("sys1", "g2", "New Caldari", -50000, 0, 0), "Add gate 2");
    assertTrue(sys.getGateCount("sys1") == 2, "2 gates");
    assertTrue(!sys.addGate("sys1", "g1", "Dup", 0, 0, 0), "Duplicate rejected");
    assertTrue(sys.useGate("sys1", "g1"), "Use gate succeeds");
    assertTrue(sys.useGate("sys1", "g1"), "Use gate again");
    assertTrue(sys.getTotalJumps("sys1") == 2, "2 total jumps");
}

static void testStarSystemManagerNPCPresence() {
    std::cout << "\n=== StarSystemManager: NPCPresence ===" << std::endl;
    ecs::World world;
    systems::StarSystemManagerSystem sys(&world);
    world.createEntity("sys1");
    sys.initialize("sys1", "Jita", 1.0f);
    assertTrue(sys.addNPCPresence("sys1", "Guristas", 5, 0.6f, true), "Add hostile NPCs");
    assertTrue(sys.addNPCPresence("sys1", "Caldari Navy", 10, 0.0f, false), "Add friendly NPCs");
    assertTrue(sys.getNPCFactionCount("sys1") == 2, "2 factions");
    assertTrue(sys.getTotalNPCShips("sys1") == 15, "15 total NPC ships");
    assertTrue(!sys.addNPCPresence("sys1", "Guristas", 3, 0.3f, true), "Duplicate faction rejected");
    assertTrue(sys.removeNPCPresence("sys1", "Guristas"), "Remove faction");
    assertTrue(sys.getNPCFactionCount("sys1") == 1, "1 faction after remove");
    assertTrue(sys.getTotalNPCShips("sys1") == 10, "10 ships after remove");
}

static void testStarSystemManagerUpdate() {
    std::cout << "\n=== StarSystemManager: Update ===" << std::endl;
    ecs::World world;
    systems::StarSystemManagerSystem sys(&world);
    world.createEntity("sys1");
    sys.initialize("sys1", "Jita", 1.0f);
    sys.update(1.0f);
    sys.update(2.5f);
    auto* entity = world.getEntity("sys1");
    auto* state = entity->getComponent<components::StarSystemState>();
    assertTrue(approxEqual(state->elapsed_time, 3.5f), "Elapsed time is 3.5");
}

static void testStarSystemManagerMaxLimits() {
    std::cout << "\n=== StarSystemManager: MaxLimits ===" << std::endl;
    ecs::World world;
    systems::StarSystemManagerSystem sys(&world);
    world.createEntity("sys1");
    sys.initialize("sys1", "Jita", 1.0f);
    auto* entity = world.getEntity("sys1");
    auto* state = entity->getComponent<components::StarSystemState>();
    state->max_celestials = 2;
    state->max_stations = 1;
    state->max_gates = 1;
    state->max_npc_factions = 1;
    sys.addCelestial("sys1", "c1", "A", "Star", 0, 0, 0, 100);
    sys.addCelestial("sys1", "c2", "B", "Planet", 1, 0, 0, 100);
    assertTrue(!sys.addCelestial("sys1", "c3", "C", "Moon", 2, 0, 0, 100), "Max celestials enforced");
    sys.addStation("sys1", "st1", "S1", "F", 0, 0, 0);
    assertTrue(!sys.addStation("sys1", "st2", "S2", "F", 0, 0, 0), "Max stations enforced");
    sys.addGate("sys1", "g1", "Dest", 0, 0, 0);
    assertTrue(!sys.addGate("sys1", "g2", "Dest2", 0, 0, 0), "Max gates enforced");
    sys.addNPCPresence("sys1", "Pirates", 5, 0.5f, true);
    assertTrue(!sys.addNPCPresence("sys1", "Navy", 10, 0.0f, false), "Max NPC factions enforced");
}

static void testStarSystemManagerSecurityClamp() {
    std::cout << "\n=== StarSystemManager: SecurityClamp ===" << std::endl;
    ecs::World world;
    systems::StarSystemManagerSystem sys(&world);
    world.createEntity("sys1");
    sys.initialize("sys1", "Lowsec", -0.5f);
    assertTrue(approxEqual(sys.getSecurityStatus("sys1"), 0.0f), "Security clamped to 0");
    world.createEntity("sys2");
    sys.initialize("sys2", "Highsec", 1.5f);
    assertTrue(approxEqual(sys.getSecurityStatus("sys2"), 1.0f), "Security clamped to 1");
}

static void testStarSystemManagerMissing() {
    std::cout << "\n=== StarSystemManager: Missing ===" << std::endl;
    ecs::World world;
    systems::StarSystemManagerSystem sys(&world);
    assertTrue(!sys.initialize("nonexistent", "X", 0.5f), "Init fails on missing");
    assertTrue(!sys.addCelestial("nonexistent", "c1", "A", "Star", 0, 0, 0, 100), "addCelestial fails");
    assertTrue(!sys.removeCelestial("nonexistent", "c1"), "removeCelestial fails");
    assertTrue(!sys.addStation("nonexistent", "s1", "A", "F", 0, 0, 0), "addStation fails");
    assertTrue(!sys.removeStation("nonexistent", "s1"), "removeStation fails");
    assertTrue(!sys.dockAtStation("nonexistent", "s1"), "dockAtStation fails");
    assertTrue(!sys.undockFromStation("nonexistent", "s1"), "undockFromStation fails");
    assertTrue(!sys.addGate("nonexistent", "g1", "D", 0, 0, 0), "addGate fails");
    assertTrue(!sys.useGate("nonexistent", "g1"), "useGate fails");
    assertTrue(!sys.addNPCPresence("nonexistent", "F", 1, 0.5f, true), "addNPCPresence fails");
    assertTrue(!sys.removeNPCPresence("nonexistent", "F"), "removeNPCPresence fails");
    assertTrue(sys.getCelestialCount("nonexistent") == 0, "0 celestials");
    assertTrue(sys.getStationCount("nonexistent") == 0, "0 stations");
    assertTrue(sys.getStationDockedCount("nonexistent", "s1") == 0, "0 docked");
    assertTrue(sys.getGateCount("nonexistent") == 0, "0 gates");
    assertTrue(sys.getTotalJumps("nonexistent") == 0, "0 jumps");
    assertTrue(sys.getNPCFactionCount("nonexistent") == 0, "0 factions");
    assertTrue(sys.getTotalNPCShips("nonexistent") == 0, "0 ships");
    assertTrue(approxEqual(sys.getSecurityStatus("nonexistent"), 0.0f), "0 security");
    assertTrue(sys.getTotalDockings("nonexistent") == 0, "0 dockings");
}


void run_star_system_manager_system_tests() {
    testStarSystemManagerCreate();
    testStarSystemManagerCelestials();
    testStarSystemManagerStations();
    testStarSystemManagerDocking();
    testStarSystemManagerDockingFull();
    testStarSystemManagerGates();
    testStarSystemManagerNPCPresence();
    testStarSystemManagerUpdate();
    testStarSystemManagerMaxLimits();
    testStarSystemManagerSecurityClamp();
    testStarSystemManagerMissing();
}
