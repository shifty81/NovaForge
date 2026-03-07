// Tests for: Safe Zone System Tests
#include "test_log.h"
#include "components/core_components.h"
#include "components/navigation_components.h"
#include "ecs/system.h"
#include "systems/safe_zone_system.h"

using namespace atlas;

// ==================== Safe Zone System Tests ====================

static void testSafeZoneCreate() {
    std::cout << "\n=== SafeZone: Create ===" << std::endl;
    ecs::World world;
    systems::SafeZoneSystem sys(&world);
    world.createEntity("zone1");
    assertTrue(sys.initialize("zone1", "sz_jita", "station_jita"), "Init succeeds");
    assertTrue(sys.getZoneState("zone1") == "Active", "State is Active by default");
    assertTrue(approxEqual(sys.getRadius("zone1"), 5000.0f), "Default radius 5000");
    assertTrue(sys.getEntitiesInside("zone1") == 0, "No entities inside");
    assertTrue(sys.areWeaponsDisabled("zone1") == true, "Weapons disabled by default");
    assertTrue(approxEqual(sys.getTetherBonus("zone1"), 0.5f), "Default tether bonus 0.5");
    assertTrue(sys.getTotalEntries("zone1") == 0, "No entries");
    assertTrue(sys.getTotalExits("zone1") == 0, "No exits");
    assertTrue(sys.getTotalWeaponsBlocked("zone1") == 0, "No weapons blocked");
}

static void testSafeZoneSetRadius() {
    std::cout << "\n=== SafeZone: SetRadius ===" << std::endl;
    ecs::World world;
    systems::SafeZoneSystem sys(&world);
    world.createEntity("zone1");
    sys.initialize("zone1", "sz_jita", "station_jita");
    assertTrue(sys.setRadius("zone1", 10000.0f), "Set radius 10000");
    assertTrue(approxEqual(sys.getRadius("zone1"), 10000.0f), "Radius is 10000");
    assertTrue(!sys.setRadius("zone1", 0.0f), "Reject zero radius");
    assertTrue(!sys.setRadius("zone1", -100.0f), "Reject negative radius");
}

static void testSafeZoneEnableDisable() {
    std::cout << "\n=== SafeZone: EnableDisable ===" << std::endl;
    ecs::World world;
    systems::SafeZoneSystem sys(&world);
    world.createEntity("zone1");
    sys.initialize("zone1", "sz_jita", "station_jita");

    assertTrue(sys.disableZone("zone1"), "Disable succeeds");
    assertTrue(sys.getZoneState("zone1") == "Disabled", "State is Disabled");
    assertTrue(sys.areWeaponsDisabled("zone1") == false, "Weapons not disabled");
    assertTrue(approxEqual(sys.getTetherBonus("zone1"), 0.0f), "No tether when disabled");

    assertTrue(sys.enableZone("zone1"), "Enable succeeds");
    assertTrue(sys.getZoneState("zone1") == "Active", "State is Active");
    assertTrue(sys.areWeaponsDisabled("zone1") == true, "Weapons disabled again");

    assertTrue(!sys.enableZone("zone1"), "Already active, enable fails");
}

static void testSafeZoneReinforce() {
    std::cout << "\n=== SafeZone: Reinforce ===" << std::endl;
    ecs::World world;
    systems::SafeZoneSystem sys(&world);
    world.createEntity("zone1");
    sys.initialize("zone1", "sz_jita", "station_jita");

    assertTrue(sys.reinforceZone("zone1"), "Reinforce succeeds");
    assertTrue(sys.getZoneState("zone1") == "Reinforced", "State is Reinforced");
    assertTrue(sys.areWeaponsDisabled("zone1") == true, "Weapons still disabled");

    sys.disableZone("zone1");
    assertTrue(!sys.reinforceZone("zone1"), "Cannot reinforce when disabled");
}

static void testSafeZoneEntityEntryExit() {
    std::cout << "\n=== SafeZone: EntityEntryExit ===" << std::endl;
    ecs::World world;
    systems::SafeZoneSystem sys(&world);
    world.createEntity("zone1");
    sys.initialize("zone1", "sz_jita", "station_jita");

    assertTrue(sys.entityEnter("zone1"), "Enter succeeds");
    assertTrue(sys.getEntitiesInside("zone1") == 1, "1 entity inside");
    assertTrue(sys.getTotalEntries("zone1") == 1, "1 total entry");

    assertTrue(sys.entityEnter("zone1"), "Second enter");
    assertTrue(sys.getEntitiesInside("zone1") == 2, "2 entities inside");

    assertTrue(sys.entityExit("zone1"), "Exit succeeds");
    assertTrue(sys.getEntitiesInside("zone1") == 1, "1 entity remaining");
    assertTrue(sys.getTotalExits("zone1") == 1, "1 total exit");

    assertTrue(sys.entityExit("zone1"), "Second exit");
    assertTrue(!sys.entityExit("zone1"), "Cannot exit when empty");
}

static void testSafeZoneEntityEntryWhenDisabled() {
    std::cout << "\n=== SafeZone: EntityEntryWhenDisabled ===" << std::endl;
    ecs::World world;
    systems::SafeZoneSystem sys(&world);
    world.createEntity("zone1");
    sys.initialize("zone1", "sz_jita", "station_jita");
    sys.disableZone("zone1");
    assertTrue(!sys.entityEnter("zone1"), "Cannot enter disabled zone");
}

static void testSafeZoneMaxEntities() {
    std::cout << "\n=== SafeZone: MaxEntities ===" << std::endl;
    ecs::World world;
    systems::SafeZoneSystem sys(&world);
    world.createEntity("zone1");
    sys.initialize("zone1", "sz_jita", "station_jita");

    auto* entity = world.getEntity("zone1");
    auto* comp = entity->getComponent<components::SafeZone>();
    comp->max_entities = 2;

    assertTrue(sys.entityEnter("zone1"), "Enter 1");
    assertTrue(sys.entityEnter("zone1"), "Enter 2");
    assertTrue(!sys.entityEnter("zone1"), "Max entities enforced");
}

static void testSafeZoneBlockWeapon() {
    std::cout << "\n=== SafeZone: BlockWeapon ===" << std::endl;
    ecs::World world;
    systems::SafeZoneSystem sys(&world);
    world.createEntity("zone1");
    sys.initialize("zone1", "sz_jita", "station_jita");

    assertTrue(sys.blockWeapon("zone1"), "Block weapon succeeds");
    assertTrue(sys.getTotalWeaponsBlocked("zone1") == 1, "1 weapon blocked");
    assertTrue(sys.blockWeapon("zone1"), "Block second weapon");
    assertTrue(sys.getTotalWeaponsBlocked("zone1") == 2, "2 weapons blocked");

    sys.disableZone("zone1");
    assertTrue(!sys.blockWeapon("zone1"), "Cannot block when weapons not disabled");
}

static void testSafeZoneUpdate() {
    std::cout << "\n=== SafeZone: Update ===" << std::endl;
    ecs::World world;
    systems::SafeZoneSystem sys(&world);
    world.createEntity("zone1");
    sys.initialize("zone1", "sz_jita", "station_jita");

    sys.update(1.0f);
    // Basic sanity — update should not crash and state remains
    assertTrue(sys.getZoneState("zone1") == "Active", "State still Active after update");
}

static void testSafeZoneMissing() {
    std::cout << "\n=== SafeZone: Missing ===" << std::endl;
    ecs::World world;
    systems::SafeZoneSystem sys(&world);
    assertTrue(!sys.initialize("nonexistent", "sz", "st"), "Init fails on missing");
    assertTrue(!sys.setRadius("nonexistent", 1000.0f), "SetRadius fails on missing");
    assertTrue(!sys.enableZone("nonexistent"), "Enable fails on missing");
    assertTrue(!sys.disableZone("nonexistent"), "Disable fails on missing");
    assertTrue(!sys.reinforceZone("nonexistent"), "Reinforce fails on missing");
    assertTrue(!sys.entityEnter("nonexistent"), "Enter fails on missing");
    assertTrue(!sys.entityExit("nonexistent"), "Exit fails on missing");
    assertTrue(!sys.blockWeapon("nonexistent"), "BlockWeapon fails on missing");
    assertTrue(sys.getZoneState("nonexistent") == "Unknown", "Unknown state on missing");
    assertTrue(sys.getRadius("nonexistent") == 0.0f, "0 radius on missing");
    assertTrue(sys.getEntitiesInside("nonexistent") == 0, "0 entities on missing");
    assertTrue(sys.areWeaponsDisabled("nonexistent") == false, "Weapons not disabled on missing");
    assertTrue(approxEqual(sys.getTetherBonus("nonexistent"), 0.0f), "0 tether on missing");
    assertTrue(sys.getTotalEntries("nonexistent") == 0, "0 entries on missing");
    assertTrue(sys.getTotalExits("nonexistent") == 0, "0 exits on missing");
    assertTrue(sys.getTotalWeaponsBlocked("nonexistent") == 0, "0 blocked on missing");
}


void run_safe_zone_system_tests() {
    testSafeZoneCreate();
    testSafeZoneSetRadius();
    testSafeZoneEnableDisable();
    testSafeZoneReinforce();
    testSafeZoneEntityEntryExit();
    testSafeZoneEntityEntryWhenDisabled();
    testSafeZoneMaxEntities();
    testSafeZoneBlockWeapon();
    testSafeZoneUpdate();
    testSafeZoneMissing();
}
