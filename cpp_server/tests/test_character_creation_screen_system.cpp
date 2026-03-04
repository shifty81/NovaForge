// Tests for: Character Creation Screen System Tests
#include "test_log.h"
#include "components/core_components.h"
#include "components/ui_components.h"
#include "ecs/system.h"
#include "systems/character_creation_screen_system.h"

using namespace atlas;

// ==================== Character Creation Screen System Tests ====================

static void testCharCreationScreenDefaults() {
    std::cout << "\n=== Char Creation Screen Defaults ===" << std::endl;
    components::CharacterCreationScreen screen;
    assertTrue(!screen.is_open, "Default not open");
    assertTrue(!screen.finalized, "Default not finalized");
    assertTrue(screen.selected_race.empty(), "No race selected");
    assertTrue(screen.selected_faction.empty(), "No faction selected");
}

static void testCharCreationScreenOpen() {
    std::cout << "\n=== Char Creation Screen Open ===" << std::endl;
    ecs::World world;
    systems::CharacterCreationScreenSystem sys(&world);

    assertTrue(sys.openScreen("player1"), "Open screen succeeds");
    assertTrue(sys.isScreenOpen("player1"), "Screen is open");
    assertTrue(!sys.openScreen("player1"), "Cannot open twice");
}

static void testCharCreationScreenRaceSelect() {
    std::cout << "\n=== Char Creation Screen Race Select ===" << std::endl;
    ecs::World world;
    systems::CharacterCreationScreenSystem sys(&world);
    sys.openScreen("player1");

    assertTrue(sys.selectRace("player1", "Synth-Born"), "Valid race accepted");
    assertTrue(sys.getSelectedRace("player1") == "Synth-Born", "Race saved correctly");
    assertTrue(!sys.selectRace("player1", "InvalidRace"), "Invalid race rejected");
}

static void testCharCreationScreenFactionSelect() {
    std::cout << "\n=== Char Creation Screen Faction Select ===" << std::endl;
    ecs::World world;
    systems::CharacterCreationScreenSystem sys(&world);
    sys.openScreen("player1");

    assertTrue(sys.selectFaction("player1", "Solari"), "Valid faction accepted");
    assertTrue(sys.getSelectedFaction("player1") == "Solari", "Faction saved correctly");
    assertTrue(!sys.selectFaction("player1", "InvalidFaction"), "Invalid faction rejected");
}

static void testCharCreationScreenSliders() {
    std::cout << "\n=== Char Creation Screen Sliders ===" << std::endl;
    ecs::World world;
    systems::CharacterCreationScreenSystem sys(&world);
    sys.openScreen("player1");

    assertTrue(sys.setAttributeSlider("player1", "strength", 0.8f), "Set strength slider");
    assertTrue(approxEqual(sys.getAttributeSlider("player1", "strength"), 0.8f), "Strength value correct");
    assertTrue(sys.setAttributeSlider("player1", "agility", 1.5f), "Set agility clamped");
    assertTrue(approxEqual(sys.getAttributeSlider("player1", "agility"), 1.0f), "Agility clamped to 1.0");
    assertTrue(!sys.setAttributeSlider("player1", "invalid_attr", 0.5f), "Invalid attribute rejected");
}

static void testCharCreationScreenValidation() {
    std::cout << "\n=== Char Creation Screen Validation ===" << std::endl;
    ecs::World world;
    systems::CharacterCreationScreenSystem sys(&world);
    sys.openScreen("player1");

    assertTrue(!sys.validateSelections("player1"), "Invalid without race/faction");
    sys.selectRace("player1", "Terran Descendant");
    assertTrue(!sys.validateSelections("player1"), "Invalid without faction");
    sys.selectFaction("player1", "Veyren");
    assertTrue(sys.validateSelections("player1"), "Valid with race and faction");
}

static void testCharCreationScreenFinalize() {
    std::cout << "\n=== Char Creation Screen Finalize ===" << std::endl;
    ecs::World world;
    systems::CharacterCreationScreenSystem sys(&world);
    sys.openScreen("player1");

    assertTrue(!sys.finalizeCharacter("player1", "TestChar"), "Cannot finalize without selections");
    sys.selectRace("player1", "Pure Alien");
    sys.selectFaction("player1", "Aurelian");
    assertTrue(!sys.finalizeCharacter("player1", ""), "Cannot finalize with empty name");
    assertTrue(sys.finalizeCharacter("player1", "TestChar"), "Finalize succeeds");
    assertTrue(!sys.isScreenOpen("player1"), "Screen closed after finalize");
}

static void testCharCreationScreenAppearance() {
    std::cout << "\n=== Char Creation Screen Appearance ===" << std::endl;
    ecs::World world;
    systems::CharacterCreationScreenSystem sys(&world);
    sys.openScreen("player1");

    assertTrue(sys.setAppearanceSlider("player1", "height", 0.7f), "Set height slider");
    assertTrue(sys.setAppearanceSlider("player1", "build", 0.3f), "Set build slider");
    assertTrue(!sys.setAppearanceSlider("player_none", "height", 0.5f), "Fails for non-existent player");
}


void run_character_creation_screen_system_tests() {
    testCharCreationScreenDefaults();
    testCharCreationScreenOpen();
    testCharCreationScreenRaceSelect();
    testCharCreationScreenFactionSelect();
    testCharCreationScreenSliders();
    testCharCreationScreenValidation();
    testCharCreationScreenFinalize();
    testCharCreationScreenAppearance();
}
