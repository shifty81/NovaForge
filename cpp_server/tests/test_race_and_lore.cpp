// Tests for: Race & Lore Tests
#include "test_log.h"
#include "components/narrative_components.h"
#include "ecs/system.h"

using namespace atlas;

// ==================== Race & Lore Tests ====================

static void testRaceDefaults() {
    std::cout << "\n=== Race Defaults ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("race1");
    auto* info = addComp<components::RaceInfo>(e);
    assertTrue(info->race == components::RaceInfo::RaceName::TerranDescendant, "Default TerranDescendant");
    assertTrue(approxEqual(info->learning_rate, 1.0f), "Default learning rate 1.0");
}

static void testRaceApplyDefaults() {
    std::cout << "\n=== Race Apply Defaults ===" << std::endl;
    ecs::World world;

    auto* e1 = world.createEntity("race_td");
    auto* td = addComp<components::RaceInfo>(e1);
    td->race = components::RaceInfo::RaceName::TerranDescendant;
    components::RaceInfo::applyRaceDefaults(*td);
    assertTrue(approxEqual(td->learning_rate, 1.2f), "TD learning rate 1.2");
    assertTrue(approxEqual(td->diplomacy_modifier, 0.15f), "TD diplomacy 0.15");

    auto* e2 = world.createEntity("race_sb");
    auto* sb = addComp<components::RaceInfo>(e2);
    sb->race = components::RaceInfo::RaceName::SynthBorn;
    components::RaceInfo::applyRaceDefaults(*sb);
    assertTrue(approxEqual(sb->automation_bonus, 0.25f), "SB automation 0.25");

    auto* e3 = world.createEntity("race_pa");
    auto* pa = addComp<components::RaceInfo>(e3);
    pa->race = components::RaceInfo::RaceName::PureAlien;
    components::RaceInfo::applyRaceDefaults(*pa);
    assertTrue(approxEqual(pa->environmental_resilience, 1.3f), "PA resilience 1.3");

    auto* e4 = world.createEntity("race_he");
    auto* he = addComp<components::RaceInfo>(e4);
    he->race = components::RaceInfo::RaceName::HybridEvolutionary;
    components::RaceInfo::applyRaceDefaults(*he);
    assertTrue(approxEqual(he->mutation_rate, 0.05f), "HE mutation 0.05");
}

static void testLoreAddEntry() {
    std::cout << "\n=== Lore Add Entry ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("lore1");
    auto* lore = addComp<components::LoreEntry>(e);
    lore->addLore("Great Calamity", "Year 3355 event", 100.0f, "ship_log");
    assertTrue(lore->getLoreCount() == 1, "One lore entry");
    assertTrue(lore->discovered_lore[0].title == "Great Calamity", "Correct title");
}

static void testLoreMaxEntries() {
    std::cout << "\n=== Lore Max Entries ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("lore2");
    auto* lore = addComp<components::LoreEntry>(e);
    lore->max_entries = 3;
    lore->addLore("A", "a", 1.0f, "ruin");
    lore->addLore("B", "b", 2.0f, "ruin");
    lore->addLore("C", "c", 3.0f, "ruin");
    lore->addLore("D", "d", 4.0f, "ruin");
    assertTrue(lore->getLoreCount() == 3, "Trimmed to max 3");
    assertTrue(lore->discovered_lore[0].title == "B", "Oldest removed");
}


void run_race_and_lore_tests() {
    testRaceDefaults();
    testRaceApplyDefaults();
    testLoreAddEntry();
    testLoreMaxEntries();
}
