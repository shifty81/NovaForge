// Tests for: Anomaly System Tests, Anomaly Visual Cue Tests
#include "test_log.h"
#include "components/mission_components.h"
#include "ecs/system.h"
#include "systems/movement_system.h"
#include "systems/anomaly_system.h"
#include <sys/stat.h>

using namespace atlas;

// ==================== Anomaly System Tests ====================

static void testAnomalyGenerateCreatesEntities() {
    ecs::World world;
    systems::AnomalySystem sys(&world);

    int count = sys.generateAnomalies("sys1", 42, 0.5f);
    assertTrue(count > 0, "generateAnomalies returns positive count");

    auto anomalies = sys.getAnomaliesInSystem("sys1");
    assertTrue(static_cast<int>(anomalies.size()) == count,
               "getAnomaliesInSystem matches generated count");
}

static void testAnomalyHighsecFewerThanNullsec() {
    ecs::World world;
    systems::AnomalySystem sys(&world);

    int highsec = sys.generateAnomalies("high", 42, 1.0f);
    int nullsec = sys.generateAnomalies("null", 42, 0.0f);
    assertTrue(nullsec > highsec,
               "Nullsec generates more anomalies than highsec");
}

static void testAnomalyDeterministicSeed() {
    ecs::World world1;
    systems::AnomalySystem sys1(&world1);
    int c1 = sys1.generateAnomalies("sys1", 12345, 0.5f);

    ecs::World world2;
    systems::AnomalySystem sys2(&world2);
    int c2 = sys2.generateAnomalies("sys1", 12345, 0.5f);

    assertTrue(c1 == c2, "Same seed produces same anomaly count");
}

static void testAnomalyDifficultyFromSecurity() {
    using D = components::Anomaly::Difficulty;
    assertTrue(systems::AnomalySystem::difficultyFromSecurity(1.0f) == D::Trivial,
               "Highsec = Trivial difficulty");
    assertTrue(systems::AnomalySystem::difficultyFromSecurity(0.5f) == D::Medium,
               "Midsec = Medium difficulty");
    assertTrue(systems::AnomalySystem::difficultyFromSecurity(0.0f) == D::Deadly,
               "Nullsec = Deadly difficulty");
}

static void testAnomalyNpcCountScales() {
    using D = components::Anomaly::Difficulty;
    int trivial = systems::AnomalySystem::npcCountFromDifficulty(D::Trivial);
    int deadly  = systems::AnomalySystem::npcCountFromDifficulty(D::Deadly);
    assertTrue(deadly > trivial, "Deadly has more NPCs than Trivial");
}

static void testAnomalyLootMultiplierScales() {
    using D = components::Anomaly::Difficulty;
    float trivial = systems::AnomalySystem::lootMultiplierFromDifficulty(D::Trivial);
    float deadly  = systems::AnomalySystem::lootMultiplierFromDifficulty(D::Deadly);
    assertTrue(deadly > trivial, "Deadly has higher loot multiplier");
}

static void testAnomalyCompleteAnomaly() {
    ecs::World world;
    systems::AnomalySystem sys(&world);

    sys.generateAnomalies("sys1", 42, 0.5f);
    auto anomalies = sys.getAnomaliesInSystem("sys1");
    assertTrue(!anomalies.empty(), "System has anomalies");

    bool completed = sys.completeAnomaly(anomalies[0]);
    assertTrue(completed, "completeAnomaly returns true");

    int after = sys.getActiveAnomalyCount("sys1");
    assertTrue(after == static_cast<int>(anomalies.size()) - 1,
               "Active count decreased by 1");
}

static void testAnomalyDespawnOnTimer() {
    ecs::World world;
    systems::AnomalySystem sys(&world);

    sys.generateAnomalies("sys1", 42, 0.5f);
    auto anomalies = sys.getAnomaliesInSystem("sys1");
    int before = static_cast<int>(anomalies.size());
    assertTrue(before > 0, "Has anomalies to despawn");

    // Tick past the despawn timer (max is 7200s)
    sys.update(8000.0f);

    int after = sys.getActiveAnomalyCount("sys1");
    assertTrue(after == 0, "All anomalies despawned after timer");
}

static void testAnomalySignatureStrength() {
    ecs::World world;
    systems::AnomalySystem sys(&world);

    sys.generateAnomalies("sys1", 42, 0.5f);
    auto anomalies = sys.getAnomaliesInSystem("sys1");
    for (const auto& id : anomalies) {
        auto* entity = world.getEntity(id);
        auto* anom = entity->getComponent<components::Anomaly>();
        assertTrue(anom->signature_strength > 0.0f && anom->signature_strength <= 1.0f,
                   "Signature strength in valid range");
    }
}


// ==================== Anomaly Visual Cue Tests ====================

static void testAnomalyVisualCueDefaults() {
    std::cout << "\n=== Anomaly Visual Cue Defaults ===" << std::endl;
    components::AnomalyVisualCue cue;
    assertTrue(cue.cue_type == components::AnomalyVisualCue::CueType::None, "Default cue type is None");
    assertTrue(approxEqual(cue.intensity, 1.0f), "Default intensity is 1.0");
    assertTrue(approxEqual(cue.radius, 500.0f), "Default radius is 500.0");
    assertTrue(cue.active, "Default active is true");
    assertTrue(approxEqual(cue.distortion_strength, 0.0f), "Default distortion is 0.0");
}

static void testAnomalyVisualCueTypeMapping() {
    std::cout << "\n=== Anomaly Visual Cue Type Mapping ===" << std::endl;
    using AT = components::Anomaly::Type;
    using CT = components::AnomalyVisualCue::CueType;

    assertTrue(systems::AnomalySystem::cueTypeFromAnomalyType(AT::Wormhole) == CT::GravityLens,
               "Wormhole maps to GravityLens");
    assertTrue(systems::AnomalySystem::cueTypeFromAnomalyType(AT::Gas) == CT::ParticleCloud,
               "Gas maps to ParticleCloud");
    assertTrue(systems::AnomalySystem::cueTypeFromAnomalyType(AT::Combat) == CT::EnergyPulse,
               "Combat maps to EnergyPulse");
    assertTrue(systems::AnomalySystem::cueTypeFromAnomalyType(AT::Mining) == CT::Shimmer,
               "Mining maps to Shimmer");
    assertTrue(systems::AnomalySystem::cueTypeFromAnomalyType(AT::Data) == CT::ElectricArc,
               "Data maps to ElectricArc");
    assertTrue(systems::AnomalySystem::cueTypeFromAnomalyType(AT::Relic) == CT::Shimmer,
               "Relic maps to Shimmer");
}


void run_anomaly_system_tests() {
    testAnomalyGenerateCreatesEntities();
    testAnomalyHighsecFewerThanNullsec();
    testAnomalyDeterministicSeed();
    testAnomalyDifficultyFromSecurity();
    testAnomalyNpcCountScales();
    testAnomalyLootMultiplierScales();
    testAnomalyCompleteAnomaly();
    testAnomalyDespawnOnTimer();
    testAnomalySignatureStrength();
    testAnomalyVisualCueDefaults();
    testAnomalyVisualCueTypeMapping();
}
