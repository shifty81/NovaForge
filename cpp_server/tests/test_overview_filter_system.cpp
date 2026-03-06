// Tests for: OverviewFilterSystem
#include "test_log.h"
#include "components/ui_components.h"
#include "ecs/system.h"
#include "systems/overview_filter_system.h"

using namespace atlas;

// ==================== OverviewFilterSystem Tests ====================

static void testOverviewAddPreset() {
    std::cout << "\n=== OverviewFilter: Add Preset ===" << std::endl;
    ecs::World world;
    systems::OverviewFilterSystem sys(&world);

    auto* e = world.createEntity("player_1");
    addComp<components::OverviewFilter>(e);

    assertTrue(sys.addPreset("player_1", "combat", "Combat"), "Add preset succeeds");
    assertTrue(sys.getPresetCount("player_1") == 1, "1 preset");
    assertTrue(!sys.addPreset("player_1", "combat", "Combat2"), "Duplicate rejected");
}

static void testOverviewRemovePreset() {
    std::cout << "\n=== OverviewFilter: Remove Preset ===" << std::endl;
    ecs::World world;
    systems::OverviewFilterSystem sys(&world);

    auto* e = world.createEntity("player_1");
    addComp<components::OverviewFilter>(e);

    sys.addPreset("player_1", "combat", "Combat");
    sys.setActivePreset("player_1", "combat");
    assertTrue(sys.removePreset("player_1", "combat"), "Remove succeeds");
    assertTrue(sys.getPresetCount("player_1") == 0, "0 presets");

    auto* ov = e->getComponent<components::OverviewFilter>();
    assertTrue(ov->active_preset_id.empty(), "Active preset cleared on delete");
}

static void testOverviewSetActivePreset() {
    std::cout << "\n=== OverviewFilter: Set Active Preset ===" << std::endl;
    ecs::World world;
    systems::OverviewFilterSystem sys(&world);

    auto* e = world.createEntity("player_1");
    addComp<components::OverviewFilter>(e);

    sys.addPreset("player_1", "travel", "Travel");
    assertTrue(sys.setActivePreset("player_1", "travel"), "Set active succeeds");
    assertTrue(!sys.setActivePreset("player_1", "nonexistent"), "Set nonexistent fails");

    auto* ov = e->getComponent<components::OverviewFilter>();
    assertTrue(ov->active_preset_id == "travel", "Active preset is travel");
}

static void testOverviewAddTypeToPreset() {
    std::cout << "\n=== OverviewFilter: Add Type To Preset ===" << std::endl;
    ecs::World world;
    systems::OverviewFilterSystem sys(&world);

    auto* e = world.createEntity("player_1");
    addComp<components::OverviewFilter>(e);

    sys.addPreset("player_1", "combat", "Combat");
    assertTrue(sys.addTypeToPreset("player_1", "combat", "Ship"), "Add Ship type");
    assertTrue(sys.addTypeToPreset("player_1", "combat", "NPC"), "Add NPC type");
    assertTrue(!sys.addTypeToPreset("player_1", "combat", "Ship"), "Duplicate type rejected");
    assertTrue(!sys.addTypeToPreset("player_1", "nonexistent", "Ship"), "Missing preset fails");
}

static void testOverviewAddEntry() {
    std::cout << "\n=== OverviewFilter: Add Entry ===" << std::endl;
    ecs::World world;
    systems::OverviewFilterSystem sys(&world);

    auto* e = world.createEntity("player_1");
    addComp<components::OverviewFilter>(e);

    assertTrue(sys.addEntry("player_1", "ship_a", "Rifter", "Ship", 5000.0f, "Hostile"), "Add entry");
    assertTrue(sys.addEntry("player_1", "ship_b", "Raven", "Ship", 15000.0f, "Friendly"), "Add entry 2");
    assertTrue(sys.getEntryCount("player_1") == 2, "2 entries");
    assertTrue(!sys.addEntry("player_1", "ship_a", "Rifter", "Ship", 5000.0f, "Hostile"),
               "Duplicate entry rejected");
}

static void testOverviewRemoveEntry() {
    std::cout << "\n=== OverviewFilter: Remove Entry ===" << std::endl;
    ecs::World world;
    systems::OverviewFilterSystem sys(&world);

    auto* e = world.createEntity("player_1");
    addComp<components::OverviewFilter>(e);

    sys.addEntry("player_1", "ship_a", "Rifter", "Ship", 5000.0f, "Hostile");
    assertTrue(sys.removeEntry("player_1", "ship_a"), "Remove succeeds");
    assertTrue(sys.getEntryCount("player_1") == 0, "0 entries");
    assertTrue(!sys.removeEntry("player_1", "ship_a"), "Remove nonexistent fails");
}

static void testOverviewSortColumn() {
    std::cout << "\n=== OverviewFilter: Sort Column ===" << std::endl;
    ecs::World world;
    systems::OverviewFilterSystem sys(&world);

    auto* e = world.createEntity("player_1");
    addComp<components::OverviewFilter>(e);

    assertTrue(sys.setSortColumn("player_1", "name", true), "Set sort to name asc");
    assertTrue(sys.setSortColumn("player_1", "distance", false), "Set sort to distance desc");
    assertTrue(!sys.setSortColumn("player_1", "invalid_col", true), "Invalid column rejected");
}

static void testOverviewFilteredCount() {
    std::cout << "\n=== OverviewFilter: Filtered Entry Count ===" << std::endl;
    ecs::World world;
    systems::OverviewFilterSystem sys(&world);

    auto* e = world.createEntity("player_1");
    addComp<components::OverviewFilter>(e);

    // Add entries of different types
    sys.addEntry("player_1", "ship_a", "Rifter", "Ship", 5000.0f, "Hostile");
    sys.addEntry("player_1", "wreck_1", "Wreck", "Wreck", 8000.0f, "Neutral");
    sys.addEntry("player_1", "npc_1", "Pirate", "NPC", 12000.0f, "Hostile");
    sys.addEntry("player_1", "gate_1", "Jump Gate", "Celestial", 50000.0f, "Neutral");

    // No active preset → all entries
    assertTrue(sys.getFilteredEntryCount("player_1") == 4, "No filter = all 4 entries");

    // Create preset that only shows Ships and NPCs
    sys.addPreset("player_1", "combat", "Combat");
    sys.addTypeToPreset("player_1", "combat", "Ship");
    sys.addTypeToPreset("player_1", "combat", "NPC");
    sys.setActivePreset("player_1", "combat");

    assertTrue(sys.getFilteredEntryCount("player_1") == 2, "Filtered to 2 (Ship + NPC)");
}

static void testOverviewDistanceFilter() {
    std::cout << "\n=== OverviewFilter: Distance Filter ===" << std::endl;
    ecs::World world;
    systems::OverviewFilterSystem sys(&world);

    auto* e = world.createEntity("player_1");
    addComp<components::OverviewFilter>(e);

    sys.addEntry("player_1", "ship_a", "Close", "Ship", 1000.0f, "Hostile");
    sys.addEntry("player_1", "ship_b", "Far", "Ship", 50000.0f, "Hostile");

    sys.addPreset("player_1", "close", "Close Range");
    sys.addTypeToPreset("player_1", "close", "Ship");
    sys.setPresetMaxDistance("player_1", "close", 10000.0f);
    sys.setActivePreset("player_1", "close");

    assertTrue(sys.getFilteredEntryCount("player_1") == 1, "Only 1 in range");
}

static void testOverviewMaxPresets() {
    std::cout << "\n=== OverviewFilter: Max Presets ===" << std::endl;
    ecs::World world;
    systems::OverviewFilterSystem sys(&world);

    auto* e = world.createEntity("player_1");
    auto* ov = addComp<components::OverviewFilter>(e);
    ov->max_presets = 3;

    sys.addPreset("player_1", "a", "A");
    sys.addPreset("player_1", "b", "B");
    sys.addPreset("player_1", "c", "C");
    assertTrue(!sys.addPreset("player_1", "d", "D"), "Max presets enforced");
    assertTrue(sys.getPresetCount("player_1") == 3, "Still 3 presets");
}

static void testOverviewMissingEntity() {
    std::cout << "\n=== OverviewFilter: Missing Entity ===" << std::endl;
    ecs::World world;
    systems::OverviewFilterSystem sys(&world);

    assertTrue(!sys.addPreset("nope", "a", "A"), "Add preset fails");
    assertTrue(!sys.addEntry("nope", "e1", "N", "Ship", 1.0f, "Neutral"), "Add entry fails");
    assertTrue(sys.getEntryCount("nope") == 0, "0 entries");
    assertTrue(sys.getPresetCount("nope") == 0, "0 presets");
    assertTrue(sys.getFilteredEntryCount("nope") == 0, "0 filtered");
    assertTrue(sys.getTotalEntriesFiltered("nope") == 0, "0 total filtered");
}

static void testOverviewSorting() {
    std::cout << "\n=== OverviewFilter: Sorting ===" << std::endl;
    ecs::World world;
    systems::OverviewFilterSystem sys(&world);

    auto* e = world.createEntity("player_1");
    auto* ov = addComp<components::OverviewFilter>(e);
    ov->update_interval = 1.0f;

    sys.addEntry("player_1", "far", "Far Ship", "Ship", 50000.0f, "Hostile");
    sys.addEntry("player_1", "close", "Close Ship", "Ship", 1000.0f, "Friendly");
    sys.addEntry("player_1", "mid", "Mid Ship", "Ship", 10000.0f, "Neutral");

    sys.setSortColumn("player_1", "distance", true);
    sys.update(1.1f);  // Trigger sort

    // After sorting ascending by distance, first entry should be closest
    assertTrue(ov->entries[0].entity_id == "close", "Closest first");
    assertTrue(ov->entries[1].entity_id == "mid", "Mid second");
    assertTrue(ov->entries[2].entity_id == "far", "Farthest last");
}

void run_overview_filter_system_tests() {
    testOverviewAddPreset();
    testOverviewRemovePreset();
    testOverviewSetActivePreset();
    testOverviewAddTypeToPreset();
    testOverviewAddEntry();
    testOverviewRemoveEntry();
    testOverviewSortColumn();
    testOverviewFilteredCount();
    testOverviewDistanceFilter();
    testOverviewMaxPresets();
    testOverviewMissingEntity();
    testOverviewSorting();
}
