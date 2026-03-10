// Tests for: CharacterPortraitSystem
#include "test_log.h"
#include "components/game_components.h"
#include "ecs/system.h"
#include "systems/character_portrait_system.h"

using namespace atlas;

// ==================== CharacterPortraitSystem Tests ====================

static void testPortraitInit() {
    std::cout << "\n=== Portrait: Init ===" << std::endl;
    ecs::World world;
    systems::CharacterPortraitSystem sys(&world);
    world.createEntity("p1");
    assertTrue(sys.initialize("p1", "Captain Nova"), "Init succeeds");
    assertTrue(sys.getPresetCount("p1") == 0, "Zero presets initially");
    assertTrue(sys.getActivePresetId("p1").empty(), "No active preset initially");
    assertTrue(sys.getCharacterName("p1") == "Captain Nova", "Character name stored");
    assertTrue(sys.getTotalUpdates("p1") == 0, "Zero updates initially");
}

static void testPortraitInitFails() {
    std::cout << "\n=== Portrait: InitFails ===" << std::endl;
    ecs::World world;
    systems::CharacterPortraitSystem sys(&world);
    assertTrue(!sys.initialize("nonexistent", "Name"), "Init fails on missing entity");
    world.createEntity("p1");
    assertTrue(!sys.initialize("p1", ""), "Init fails with empty name");
}

static void testPortraitAddPreset() {
    std::cout << "\n=== Portrait: AddPreset ===" << std::endl;
    ecs::World world;
    systems::CharacterPortraitSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1", "Alice");

    assertTrue(sys.addPreset("p1", "pr1", "Default", "nebula", "soft",
               "neutral", "calm", 0.0f), "Add first preset");
    assertTrue(sys.addPreset("p1", "pr2", "Combat", "station", "dramatic",
               "aggressive", "fierce", 15.0f), "Add second preset");
    assertTrue(sys.getPresetCount("p1") == 2, "Two presets stored");
    assertTrue(!sys.addPreset("p1", "pr1", "Duplicate", "bg", "lt",
               "ps", "ex", 0.0f), "Duplicate preset_id rejected");
    assertTrue(!sys.addPreset("p1", "", "Empty ID", "bg", "lt",
               "ps", "ex", 0.0f), "Empty preset_id rejected");
    assertTrue(!sys.addPreset("p1", "pr3", "", "bg", "lt",
               "ps", "ex", 0.0f), "Empty name rejected");
    assertTrue(sys.getPresetCount("p1") == 2, "Count unchanged after rejections");
}

static void testPortraitRemovePreset() {
    std::cout << "\n=== Portrait: RemovePreset ===" << std::endl;
    ecs::World world;
    systems::CharacterPortraitSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1", "Alice");
    sys.addPreset("p1", "pr1", "Default", "nebula", "soft", "neutral", "calm", 0.0f);
    sys.addPreset("p1", "pr2", "Combat", "station", "dramatic", "aggressive", "fierce", 15.0f);

    assertTrue(sys.removePreset("p1", "pr1"), "Remove existing preset");
    assertTrue(sys.getPresetCount("p1") == 1, "Count decremented");
    assertTrue(!sys.hasPreset("p1", "pr1"), "Removed preset no longer found");
    assertTrue(!sys.removePreset("p1", "pr1"), "Remove nonexistent fails");
    assertTrue(!sys.removePreset("p1", "nonexistent"), "Remove unknown fails");
}

static void testPortraitSetActive() {
    std::cout << "\n=== Portrait: SetActive ===" << std::endl;
    ecs::World world;
    systems::CharacterPortraitSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1", "Alice");
    sys.addPreset("p1", "pr1", "Default", "nebula", "soft", "neutral", "calm", 0.0f);
    sys.addPreset("p1", "pr2", "Combat", "station", "dramatic", "aggressive", "fierce", 15.0f);

    assertTrue(sys.setActivePortrait("p1", "pr1"), "Set active to pr1");
    assertTrue(sys.getActivePresetId("p1") == "pr1", "pr1 is active");
    assertTrue(sys.setActivePortrait("p1", "pr2"), "Switch active to pr2");
    assertTrue(sys.getActivePresetId("p1") == "pr2", "pr2 is now active");
    assertTrue(!sys.setActivePortrait("p1", "nonexistent"), "Cannot set nonexistent active");
    assertTrue(sys.getActivePresetId("p1") == "pr2", "Still pr2 after failed switch");
}

static void testPortraitEditActive() {
    std::cout << "\n=== Portrait: EditActive ===" << std::endl;
    ecs::World world;
    systems::CharacterPortraitSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1", "Alice");
    sys.addPreset("p1", "pr1", "Default", "nebula", "soft", "neutral", "calm", 0.0f);
    sys.setActivePortrait("p1", "pr1");

    assertTrue(sys.setBackground("p1", "galaxy"), "Set background");
    assertTrue(sys.setLighting("p1", "harsh"), "Set lighting");
    assertTrue(sys.setPose("p1", "heroic"), "Set pose");
    assertTrue(sys.setExpression("p1", "determined"), "Set expression");
    assertTrue(sys.setCameraAngle("p1", 45.0f), "Set camera angle");
    assertTrue(sys.getTotalUpdates("p1") == 5, "5 updates recorded");
}

static void testPortraitEditNoActive() {
    std::cout << "\n=== Portrait: EditNoActive ===" << std::endl;
    ecs::World world;
    systems::CharacterPortraitSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1", "Alice");
    sys.addPreset("p1", "pr1", "Default", "nebula", "soft", "neutral", "calm", 0.0f);
    // No active portrait set

    assertTrue(!sys.setBackground("p1", "galaxy"), "Cannot edit without active preset");
    assertTrue(!sys.setLighting("p1", "harsh"), "Cannot edit lighting without active");
    assertTrue(!sys.setPose("p1", "heroic"), "Cannot edit pose without active");
    assertTrue(!sys.setExpression("p1", "determined"), "Cannot edit expression without active");
    assertTrue(!sys.setCameraAngle("p1", 45.0f), "Cannot edit angle without active");
    assertTrue(sys.getTotalUpdates("p1") == 0, "Zero updates when no active preset");
}

static void testPortraitRemoveActive() {
    std::cout << "\n=== Portrait: RemoveActive ===" << std::endl;
    ecs::World world;
    systems::CharacterPortraitSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1", "Alice");
    sys.addPreset("p1", "pr1", "Default", "nebula", "soft", "neutral", "calm", 0.0f);
    sys.setActivePortrait("p1", "pr1");
    assertTrue(sys.getActivePresetId("p1") == "pr1", "pr1 is active");

    assertTrue(sys.removePreset("p1", "pr1"), "Remove active preset succeeds");
    assertTrue(sys.getActivePresetId("p1").empty(), "No active preset after removal");
    assertTrue(sys.getPresetCount("p1") == 0, "Zero presets after removal");
}

static void testPortraitMaxCapacity() {
    std::cout << "\n=== Portrait: MaxCapacity ===" << std::endl;
    ecs::World world;
    systems::CharacterPortraitSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1", "Alice");

    // Set small max for testing
    auto* comp = world.getEntity("p1")->getComponent<components::CharacterPortrait>();
    comp->max_presets = 3;

    sys.addPreset("p1", "p1a", "A", "bg", "lt", "ps", "ex", 0.0f);
    sys.addPreset("p1", "p2a", "B", "bg", "lt", "ps", "ex", 0.0f);
    sys.addPreset("p1", "p3a", "C", "bg", "lt", "ps", "ex", 0.0f);
    assertTrue(sys.getPresetCount("p1") == 3, "Three presets at capacity");
    assertTrue(!sys.addPreset("p1", "p4a", "D", "bg", "lt", "ps", "ex", 0.0f),
               "Fourth preset rejected at capacity");
    assertTrue(sys.getPresetCount("p1") == 3, "Count still 3");
}

static void testPortraitHasPreset() {
    std::cout << "\n=== Portrait: HasPreset ===" << std::endl;
    ecs::World world;
    systems::CharacterPortraitSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1", "Alice");
    sys.addPreset("p1", "pr1", "Default", "nebula", "soft", "neutral", "calm", 0.0f);

    assertTrue(sys.hasPreset("p1", "pr1"), "Has pr1");
    assertTrue(!sys.hasPreset("p1", "nonexistent"), "Does not have nonexistent");
    assertTrue(!sys.hasPreset("nonexistent", "pr1"), "Missing entity returns false");
}

static void testPortraitMissing() {
    std::cout << "\n=== Portrait: Missing ===" << std::endl;
    ecs::World world;
    systems::CharacterPortraitSystem sys(&world);

    assertTrue(!sys.addPreset("nonexistent", "p", "P", "b", "l", "ps", "e", 0.0f),
               "AddPreset fails on missing");
    assertTrue(!sys.removePreset("nonexistent", "p"), "RemovePreset fails on missing");
    assertTrue(!sys.setActivePortrait("nonexistent", "p"), "SetActive fails on missing");
    assertTrue(!sys.setBackground("nonexistent", "bg"), "SetBackground fails on missing");
    assertTrue(!sys.setLighting("nonexistent", "lt"), "SetLighting fails on missing");
    assertTrue(!sys.setPose("nonexistent", "ps"), "SetPose fails on missing");
    assertTrue(!sys.setExpression("nonexistent", "ex"), "SetExpression fails on missing");
    assertTrue(!sys.setCameraAngle("nonexistent", 0.0f), "SetCameraAngle fails on missing");
    assertTrue(sys.getPresetCount("nonexistent") == 0, "Zero presets on missing");
    assertTrue(sys.getActivePresetId("nonexistent").empty(), "Empty active on missing");
    assertTrue(sys.getCharacterName("nonexistent").empty(), "Empty name on missing");
    assertTrue(sys.getTotalUpdates("nonexistent") == 0, "Zero updates on missing");
    assertTrue(!sys.hasPreset("nonexistent", "p"), "HasPreset false on missing");
}

void run_character_portrait_system_tests() {
    testPortraitInit();
    testPortraitInitFails();
    testPortraitAddPreset();
    testPortraitRemovePreset();
    testPortraitSetActive();
    testPortraitEditActive();
    testPortraitEditNoActive();
    testPortraitRemoveActive();
    testPortraitMaxCapacity();
    testPortraitHasPreset();
    testPortraitMissing();
}
