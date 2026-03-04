// Tests for: CaptainBackgroundSystem Tests
#include "test_log.h"
#include "components/fleet_components.h"
#include "ecs/system.h"
#include "systems/captain_background_system.h"

using namespace atlas;

// ==================== CaptainBackgroundSystem Tests ====================

static void testCaptainBgAssign() {
    std::cout << "\n=== Captain Background: Assign ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("captain1");
    addComp<components::CaptainBackground>(e);

    systems::CaptainBackgroundSystem sys(&world);
    assertTrue(sys.assignBackground("captain1", "ExMilitary"), "Background assigned");
    assertTrue(sys.getBackground("captain1") == "ExMilitary", "Background is ExMilitary");
    assertTrue(sys.getPreferredRole("captain1") == "Combat", "ExMilitary prefers Combat");
    assertTrue(sys.getDialogueFlavor("captain1") == "formal", "ExMilitary speaks formally");
}

static void testCaptainBgFormerMiner() {
    std::cout << "\n=== Captain Background: Former Miner ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("captain1");
    addComp<components::CaptainBackground>(e);

    systems::CaptainBackgroundSystem sys(&world);
    sys.assignBackground("captain1", "FormerMiner");
    assertTrue(sys.getPreferredRole("captain1") == "Mining", "FormerMiner prefers Mining");
    assertTrue(sys.getDialogueFlavor("captain1") == "gruff", "FormerMiner is gruff");
    assertTrue(sys.getSkillCategory("captain1") == "mining", "Skill category is mining");
    assertTrue(sys.getSkillBonus("captain1") > 0.0f, "Has skill bonus");
    assertTrue(sys.getAggressionModifier("captain1") < 0.0f, "Miners are less aggressive");
}

static void testCaptainBgSmuggler() {
    std::cout << "\n=== Captain Background: Smuggler ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("captain1");
    addComp<components::CaptainBackground>(e);

    systems::CaptainBackgroundSystem sys(&world);
    sys.assignBackground("captain1", "Smuggler");
    assertTrue(sys.getPreferredRole("captain1") == "Scout", "Smuggler prefers Scout");
    assertTrue(sys.getDialogueFlavor("captain1") == "sly", "Smuggler is sly");
    assertTrue(sys.getLoyaltyModifier("captain1") < 0.0f, "Smugglers have lower loyalty");
}

static void testCaptainBgScientist() {
    std::cout << "\n=== Captain Background: Scientist ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("captain1");
    addComp<components::CaptainBackground>(e);

    systems::CaptainBackgroundSystem sys(&world);
    sys.assignBackground("captain1", "Scientist");
    assertTrue(sys.getPreferredRole("captain1") == "Support", "Scientist prefers Support");
    assertTrue(sys.getDialogueFlavor("captain1") == "analytical", "Scientist is analytical");
    assertTrue(sys.getSkillCategory("captain1") == "exploration", "Skill category is exploration");
    assertTrue(sys.getAggressionModifier("captain1") < 0.0f, "Scientists are less aggressive");
}

static void testCaptainBgGenerate() {
    std::cout << "\n=== Captain Background: Generate from Seed ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("captain1");
    addComp<components::CaptainBackground>(e);

    systems::CaptainBackgroundSystem sys(&world);
    assertTrue(sys.generateBackground("captain1", 42), "Generated background");
    std::string bg = sys.getBackground("captain1");
    assertTrue(bg != "Unknown", "Valid background generated");
    assertTrue(!sys.getPreferredRole("captain1").empty(), "Has preferred role");
    assertTrue(!sys.getDialogueFlavor("captain1").empty(), "Has dialogue flavor");
    assertTrue(sys.getExperience("captain1") > 0, "Has experience years");
    assertTrue(!sys.getOriginSystem("captain1").empty(), "Has origin system");
}

static void testCaptainBgDeterministic() {
    std::cout << "\n=== Captain Background: Deterministic ===" << std::endl;
    ecs::World world;
    auto* e1 = world.createEntity("captain1");
    auto* e2 = world.createEntity("captain2");
    addComp<components::CaptainBackground>(e1);
    addComp<components::CaptainBackground>(e2);

    systems::CaptainBackgroundSystem sys(&world);
    sys.generateBackground("captain1", 12345);
    sys.generateBackground("captain2", 12345);
    assertTrue(sys.getBackground("captain1") == sys.getBackground("captain2"), "Same seed = same background");
    assertTrue(sys.getExperience("captain1") == sys.getExperience("captain2"), "Same seed = same experience");
    assertTrue(sys.getOriginSystem("captain1") == sys.getOriginSystem("captain2"), "Same seed = same origin");
}

static void testCaptainBgSetOrigin() {
    std::cout << "\n=== Captain Background: Set Origin ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("captain1");
    addComp<components::CaptainBackground>(e);

    systems::CaptainBackgroundSystem sys(&world);
    assertTrue(sys.setOriginSystem("captain1", "Solari Prime"), "Origin set");
    assertTrue(sys.getOriginSystem("captain1") == "Solari Prime", "Origin is Solari Prime");
}

static void testCaptainBgSetExperience() {
    std::cout << "\n=== Captain Background: Set Experience ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("captain1");
    addComp<components::CaptainBackground>(e);

    systems::CaptainBackgroundSystem sys(&world);
    assertTrue(sys.setExperience("captain1", 15), "Experience set");
    assertTrue(sys.getExperience("captain1") == 15, "Experience is 15 years");
}

static void testCaptainBgAllTypes() {
    std::cout << "\n=== Captain Background: All Types ===" << std::endl;
    ecs::World world;
    systems::CaptainBackgroundSystem sys(&world);
    const char* types[] = {"FormerMiner", "ExMilitary", "Smuggler", "Scientist",
                           "Noble", "Colonist", "BountyHunter", "Trader"};
    for (int i = 0; i < 8; ++i) {
        std::string id = "captain_" + std::to_string(i);
        auto* e = world.createEntity(id);
        addComp<components::CaptainBackground>(e);
        sys.assignBackground(id, types[i]);
        assertTrue(!sys.getPreferredRole(id).empty(), std::string(types[i]) + " has preferred role");
        assertTrue(!sys.getDialogueFlavor(id).empty(), std::string(types[i]) + " has dialogue flavor");
        assertTrue(sys.getSkillBonus(id) > 0.0f, std::string(types[i]) + " has skill bonus");
    }
}

static void testCaptainBgMissing() {
    std::cout << "\n=== Captain Background: Missing Entity ===" << std::endl;
    ecs::World world;
    systems::CaptainBackgroundSystem sys(&world);
    assertTrue(sys.getBackground("nonexistent") == "Unknown", "Default background Unknown");
    assertTrue(sys.getPreferredRole("nonexistent").empty(), "Default role empty");
    assertTrue(sys.getDialogueFlavor("nonexistent").empty(), "Default flavor empty");
    assertTrue(approxEqual(sys.getSkillBonus("nonexistent"), 0.0f), "Default skill bonus 0");
    assertTrue(sys.getSkillCategory("nonexistent").empty(), "Default skill category empty");
    assertTrue(approxEqual(sys.getAggressionModifier("nonexistent"), 0.0f), "Default aggression mod 0");
    assertTrue(approxEqual(sys.getLoyaltyModifier("nonexistent"), 0.0f), "Default loyalty mod 0");
    assertTrue(sys.getExperience("nonexistent") == 0, "Default experience 0");
    assertTrue(sys.getOriginSystem("nonexistent").empty(), "Default origin empty");
}


void run_captain_background_system_tests() {
    testCaptainBgAssign();
    testCaptainBgFormerMiner();
    testCaptainBgSmuggler();
    testCaptainBgScientist();
    testCaptainBgGenerate();
    testCaptainBgDeterministic();
    testCaptainBgSetOrigin();
    testCaptainBgSetExperience();
    testCaptainBgAllTypes();
    testCaptainBgMissing();
}
