// Tests for: Ancient AI Remnant System Tests
#include "test_log.h"
#include "components/exploration_components.h"
#include "ecs/system.h"
#include "systems/ancient_ai_remnant_system.h"
#include "systems/movement_system.h"

using namespace atlas;

// ==================== Ancient AI Remnant System Tests ====================

static void testAncientAIRemnantDefaults() {
    std::cout << "\n=== Ancient AI Remnant Defaults ===" << std::endl;
    components::AncientAIRemnant remnant;
    assertTrue(remnant.active == true, "Default active");
    assertTrue(remnant.defeated == false, "Default not defeated");
    assertTrue(remnant.tier == 1, "Default tier 1");
    assertTrue(remnant.remnant_type == components::AncientAIRemnant::RemnantType::Sentinel, "Default Sentinel type");
    assertTrue(remnant.isActive(), "Active by default");
}

static void testAncientAIRemnantSpawn() {
    std::cout << "\n=== Ancient AI Remnant Spawn ===" << std::endl;
    ecs::World world;
    systems::AncientAIRemnantSystem sys(&world);

    std::string id = sys.spawnRemnant("ancient_site_1", 3);
    assertTrue(!id.empty(), "Remnant ID generated");
    assertTrue(sys.isRemnantActive(id), "Remnant is active after spawn");
    assertTrue(sys.getActiveRemnantCount() == 1, "One active remnant");
}

static void testAncientAIRemnantTierScaling() {
    std::cout << "\n=== Ancient AI Remnant Tier Scaling ===" << std::endl;
    ecs::World world;
    systems::AncientAIRemnantSystem sys(&world);

    std::string id1 = sys.spawnRemnant("site_1", 1);
    std::string id5 = sys.spawnRemnant("site_5", 5);

    float diff1 = sys.getRemnantDifficulty(id1);
    float diff5 = sys.getRemnantDifficulty(id5);
    assertTrue(diff5 > diff1, "Higher tier = higher difficulty");
    assertTrue(approxEqual(diff1, 1.0f), "Tier 1 difficulty is 1.0");
    assertTrue(approxEqual(diff5, 5.0f), "Tier 5 difficulty is 5.0");
}

static void testAncientAIRemnantDefeat() {
    std::cout << "\n=== Ancient AI Remnant Defeat ===" << std::endl;
    ecs::World world;
    systems::AncientAIRemnantSystem sys(&world);

    std::string id = sys.spawnRemnant("ancient_site_1", 2);
    assertTrue(sys.isRemnantActive(id), "Active before defeat");
    assertTrue(sys.defeatRemnant(id), "Defeat succeeds");
    assertTrue(!sys.isRemnantActive(id), "Inactive after defeat");
    assertTrue(sys.getActiveRemnantCount() == 0, "Zero active after defeat");
}

static void testAncientAIRemnantExpiry() {
    std::cout << "\n=== Ancient AI Remnant Expiry ===" << std::endl;
    ecs::World world;
    systems::AncientAIRemnantSystem sys(&world);

    std::string id = sys.spawnRemnant("ancient_site_1", 1);
    // Default max_duration is 7200s. Advance time past that.
    sys.update(7201.0f);
    assertTrue(!sys.isRemnantActive(id), "Inactive after expiry");
}

static void testAncientAIRemnantSiteId() {
    std::cout << "\n=== Ancient AI Remnant Site ID ===" << std::endl;
    ecs::World world;
    systems::AncientAIRemnantSystem sys(&world);

    std::string id = sys.spawnRemnant("ancient_ruins_alpha", 4);
    assertTrue(sys.getRemnantSiteId(id) == "ancient_ruins_alpha", "Site ID matches");
}

static void testAncientAIRemnantTypeName() {
    std::cout << "\n=== Ancient AI Remnant Type Name ===" << std::endl;
    assertTrue(systems::AncientAIRemnantSystem::getRemnantTypeName(0) == "Sentinel", "Type 0 is Sentinel");
    assertTrue(systems::AncientAIRemnantSystem::getRemnantTypeName(1) == "Swarm", "Type 1 is Swarm");
    assertTrue(systems::AncientAIRemnantSystem::getRemnantTypeName(2) == "Construct", "Type 2 is Construct");
    assertTrue(systems::AncientAIRemnantSystem::getRemnantTypeName(3) == "Warden", "Type 3 is Warden");
    assertTrue(systems::AncientAIRemnantSystem::getRemnantTypeName(4) == "Leviathan", "Type 4 is Leviathan");
    assertTrue(systems::AncientAIRemnantSystem::getRemnantTypeName(99) == "Unknown", "Invalid type is Unknown");
}


void run_ancient_ai_remnant_system_tests() {
    testAncientAIRemnantDefaults();
    testAncientAIRemnantSpawn();
    testAncientAIRemnantTierScaling();
    testAncientAIRemnantDefeat();
    testAncientAIRemnantExpiry();
    testAncientAIRemnantSiteId();
    testAncientAIRemnantTypeName();
}
