// Tests for: Meta Level System
#include "test_log.h"
#include "components/core_components.h"
#include "components/ship_components.h"
#include "ecs/system.h"
#include "systems/meta_level_system.h"

using namespace atlas;

// ==================== Meta Level System Tests ====================

static void testMetaLevelCreate() {
    std::cout << "\n=== MetaLevel: Create ===" << std::endl;
    ecs::World world;
    systems::MetaLevelSystem sys(&world);
    world.createEntity("fit1");
    assertTrue(sys.initialize("fit1", "fitting_001"), "Init succeeds");
    assertTrue(sys.getModuleCount("fit1") == 0, "No modules initially");
    assertTrue(approxEqual(sys.getAverageMetaLevel("fit1"), 0.0f), "0 avg meta level");
    assertTrue(sys.getTechIICount("fit1") == 0, "0 Tech II");
    assertTrue(sys.getFactionCount("fit1") == 0, "0 Faction");
}

static void testMetaLevelAddModule() {
    std::cout << "\n=== MetaLevel: AddModule ===" << std::endl;
    ecs::World world;
    systems::MetaLevelSystem sys(&world);
    world.createEntity("fit1");
    sys.initialize("fit1", "fitting_001");

    assertTrue(sys.addModule("fit1", "mod1", "Shield Booster", 0),
               "Add T1 module");
    assertTrue(sys.getModuleCount("fit1") == 1, "1 module");
    assertTrue(sys.getMetaLevel("fit1", "mod1") == 0, "Meta level 0");

    assertTrue(sys.addModule("fit1", "mod2", "Armor Repairer", 5, 1.2f, 1.1f, 1.15f),
               "Add T2 module");
    assertTrue(sys.getModuleCount("fit1") == 2, "2 modules");
    assertTrue(sys.getMetaLevel("fit1", "mod2") == 5, "Meta level 5");
}

static void testMetaLevelRemoveModule() {
    std::cout << "\n=== MetaLevel: RemoveModule ===" << std::endl;
    ecs::World world;
    systems::MetaLevelSystem sys(&world);
    world.createEntity("fit1");
    sys.initialize("fit1");
    sys.addModule("fit1", "mod1", "Gyrostabilizer", 3);
    sys.addModule("fit1", "mod2", "Damage Control", 0);

    assertTrue(sys.removeModule("fit1", "mod1"), "Remove succeeds");
    assertTrue(sys.getModuleCount("fit1") == 1, "1 module remaining");
    assertTrue(!sys.removeModule("fit1", "mod1"), "Double remove fails");
    assertTrue(sys.getMetaLevel("fit1", "mod1") == -1, "Removed module not found");
}

static void testMetaLevelUpgrade() {
    std::cout << "\n=== MetaLevel: Upgrade ===" << std::endl;
    ecs::World world;
    systems::MetaLevelSystem sys(&world);
    world.createEntity("fit1");
    sys.initialize("fit1");
    sys.addModule("fit1", "mod1", "Shield Booster", 0, 1.0f, 1.0f, 1.0f);

    assertTrue(sys.upgradeModule("fit1", "mod1", 5), "Upgrade to T2");
    assertTrue(sys.getMetaLevel("fit1", "mod1") == 5, "Meta level now 5");
    // Multipliers should have increased (factor = 1.0 + 0.05*5 = 1.25)
    assertTrue(sys.getStatMultiplier("fit1", "mod1") > 1.0f, "Stat mult increased");

    assertTrue(!sys.upgradeModule("fit1", "nonexistent", 5), "Upgrade missing fails");
}

static void testMetaLevelStatMultipliers() {
    std::cout << "\n=== MetaLevel: StatMultipliers ===" << std::endl;
    ecs::World world;
    systems::MetaLevelSystem sys(&world);
    world.createEntity("fit1");
    sys.initialize("fit1");
    sys.addModule("fit1", "mod1", "Webifier", 3, 1.5f, 1.2f, 0.9f);

    assertTrue(approxEqual(sys.getStatMultiplier("fit1", "mod1"), 1.5f), "Stat 1.5");
    assertTrue(approxEqual(sys.getCPUMultiplier("fit1", "mod1"), 1.2f), "CPU 1.2");
    assertTrue(approxEqual(sys.getPowerGridMultiplier("fit1", "mod1"), 0.9f), "PG 0.9");

    // Missing module returns 0
    assertTrue(approxEqual(sys.getStatMultiplier("fit1", "none"), 0.0f),
               "Missing stat 0");
    assertTrue(approxEqual(sys.getCPUMultiplier("fit1", "none"), 0.0f),
               "Missing cpu 0");
    assertTrue(approxEqual(sys.getPowerGridMultiplier("fit1", "none"), 0.0f),
               "Missing pg 0");
}

static void testMetaLevelTechIICount() {
    std::cout << "\n=== MetaLevel: TechIICount ===" << std::endl;
    ecs::World world;
    systems::MetaLevelSystem sys(&world);
    world.createEntity("fit1");
    sys.initialize("fit1");

    sys.addModule("fit1", "mod1", "Shield Booster", 5, 1.2f, 1.1f, 1.1f);
    sys.addModule("fit1", "mod2", "Armor Repairer", 5, 1.2f, 1.1f, 1.1f);
    sys.addModule("fit1", "mod3", "Gyrostabilizer", 0);
    sys.addModule("fit1", "mod4", "Damage Control", 7);

    assertTrue(sys.getTechIICount("fit1") == 2, "2 Tech II modules");
}

static void testMetaLevelFactionCount() {
    std::cout << "\n=== MetaLevel: FactionCount ===" << std::endl;
    ecs::World world;
    systems::MetaLevelSystem sys(&world);
    world.createEntity("fit1");
    sys.initialize("fit1");

    sys.addModule("fit1", "mod1", "Shield Booster", 6, 1.5f, 1.3f, 1.2f);
    sys.addModule("fit1", "mod2", "Armor Repairer", 8, 2.0f, 1.5f, 1.4f);
    sys.addModule("fit1", "mod3", "Gyrostabilizer", 5, 1.2f, 1.1f, 1.1f);
    sys.addModule("fit1", "mod4", "Damage Control", 0);

    assertTrue(sys.getFactionCount("fit1") == 2, "2 Faction+ modules");
}

static void testMetaLevelAverageLevel() {
    std::cout << "\n=== MetaLevel: AverageLevel ===" << std::endl;
    ecs::World world;
    systems::MetaLevelSystem sys(&world);
    world.createEntity("fit1");
    sys.initialize("fit1");

    sys.addModule("fit1", "mod1", "Shield Booster", 0);
    sys.addModule("fit1", "mod2", "Armor Repairer", 5);
    sys.addModule("fit1", "mod3", "Gyrostabilizer", 5);
    // Average = (0 + 5 + 5) / 3 ≈ 3.333
    float avg = sys.getAverageMetaLevel("fit1");
    assertTrue(avg > 3.3f && avg < 3.4f, "Average ~3.33");
}

static void testMetaLevelMaxModules() {
    std::cout << "\n=== MetaLevel: MaxModules ===" << std::endl;
    ecs::World world;
    systems::MetaLevelSystem sys(&world);
    world.createEntity("fit1");
    sys.initialize("fit1");

    for (int i = 0; i < 16; i++) {
        assertTrue(sys.addModule("fit1", "mod" + std::to_string(i), "Generic", 0),
                   "Add module " + std::to_string(i));
    }
    assertTrue(sys.getModuleCount("fit1") == 16, "16 modules (max)");
    assertTrue(!sys.addModule("fit1", "overflow", "Generic", 0), "Overflow rejected");
}

static void testMetaLevelMissing() {
    std::cout << "\n=== MetaLevel: Missing ===" << std::endl;
    ecs::World world;
    systems::MetaLevelSystem sys(&world);
    assertTrue(!sys.initialize("nonexistent"), "Init fails on missing");
    assertTrue(!sys.addModule("nonexistent", "m1", "Generic", 0),
               "Add fails on missing");
    assertTrue(!sys.removeModule("nonexistent", "m1"), "Remove fails on missing");
    assertTrue(!sys.upgradeModule("nonexistent", "m1", 5), "Upgrade fails on missing");
    assertTrue(!sys.setDropRate("nonexistent", "m1", 0.5f), "SetDrop fails on missing");
    assertTrue(sys.getModuleCount("nonexistent") == 0, "0 modules on missing");
    assertTrue(sys.getMetaLevel("nonexistent", "m1") == -1, "Meta -1 on missing");
    assertTrue(approxEqual(sys.getAverageMetaLevel("nonexistent"), 0.0f),
               "0 avg on missing");
    assertTrue(approxEqual(sys.getStatMultiplier("nonexistent", "m1"), 0.0f),
               "0 stat on missing");
    assertTrue(approxEqual(sys.getCPUMultiplier("nonexistent", "m1"), 0.0f),
               "0 cpu on missing");
    assertTrue(approxEqual(sys.getPowerGridMultiplier("nonexistent", "m1"), 0.0f),
               "0 pg on missing");
    assertTrue(sys.getTechIICount("nonexistent") == 0, "0 T2 on missing");
    assertTrue(sys.getFactionCount("nonexistent") == 0, "0 faction on missing");
}

void run_meta_level_system_tests() {
    testMetaLevelCreate();
    testMetaLevelAddModule();
    testMetaLevelRemoveModule();
    testMetaLevelUpgrade();
    testMetaLevelStatMultipliers();
    testMetaLevelTechIICount();
    testMetaLevelFactionCount();
    testMetaLevelAverageLevel();
    testMetaLevelMaxModules();
    testMetaLevelMissing();
}
