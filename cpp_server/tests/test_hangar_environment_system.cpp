// Tests for: HangarEnvironmentSystem
#include "test_log.h"
#include "components/ship_components.h"
#include "ecs/system.h"
#include "systems/hangar_environment_system.h"

using namespace atlas;

// ==================== HangarEnvironmentSystem Tests ====================

static void testHangarEnvInit() {
    std::cout << "\n=== HangarEnvironment: Init ===" << std::endl;
    ecs::World world;
    systems::HangarEnvironmentSystem sys(&world);
    world.createEntity("h1");

    using AT = components::HangarEnvironment::AtmosphereType;
    assertTrue(sys.initializeEnvironment("h1", AT::Breathable, 22.0f, 1.0f), "Init succeeds");
    assertTrue(sys.getToxicity("h1") == 0.0f, "Initial toxicity is 0");
    assertTrue(!sys.isAlarmActive("h1"), "Alarm inactive initially");
    assertTrue(sys.getOccupantCount("h1") == 0, "Zero occupants initially");
}

static void testHangarEnvInitDuplicate() {
    std::cout << "\n=== HangarEnvironment: InitDuplicate ===" << std::endl;
    ecs::World world;
    systems::HangarEnvironmentSystem sys(&world);
    world.createEntity("h1");

    using AT = components::HangarEnvironment::AtmosphereType;
    assertTrue(sys.initializeEnvironment("h1", AT::Breathable, 22.0f, 1.0f), "First init ok");
    assertTrue(!sys.initializeEnvironment("h1", AT::Toxic, 50.0f, 0.5f), "Duplicate init fails");
}

static void testHangarEnvInitMissing() {
    std::cout << "\n=== HangarEnvironment: InitMissing ===" << std::endl;
    ecs::World world;
    systems::HangarEnvironmentSystem sys(&world);

    using AT = components::HangarEnvironment::AtmosphereType;
    assertTrue(!sys.initializeEnvironment("missing", AT::Breathable, 22.0f, 1.0f), "Missing entity fails");
}

static void testHangarEnvOpenClose() {
    std::cout << "\n=== HangarEnvironment: OpenClose ===" << std::endl;
    ecs::World world;
    systems::HangarEnvironmentSystem sys(&world);
    world.createEntity("h1");

    using AT = components::HangarEnvironment::AtmosphereType;
    sys.initializeEnvironment("h1", AT::Breathable, 22.0f, 1.0f);

    assertTrue(sys.openHangar("h1"), "Open succeeds");
    assertTrue(sys.closeHangar("h1"), "Close succeeds");
    assertTrue(!sys.openHangar("missing"), "Open missing fails");
    assertTrue(!sys.closeHangar("missing"), "Close missing fails");
}

static void testHangarEnvOccupants() {
    std::cout << "\n=== HangarEnvironment: Occupants ===" << std::endl;
    ecs::World world;
    systems::HangarEnvironmentSystem sys(&world);
    world.createEntity("h1");

    using AT = components::HangarEnvironment::AtmosphereType;
    sys.initializeEnvironment("h1", AT::Breathable, 22.0f, 1.0f);

    assertTrue(sys.addOccupant("h1", "crew1", false, 0.0f), "Add crew1");
    assertTrue(sys.getOccupantCount("h1") == 1, "1 occupant");
    assertTrue(sys.addOccupant("h1", "crew2", true, 0.8f), "Add crew2 with suit");
    assertTrue(sys.getOccupantCount("h1") == 2, "2 occupants");

    // Duplicate rejected
    assertTrue(!sys.addOccupant("h1", "crew1", false, 0.0f), "Duplicate rejected");
    assertTrue(sys.getOccupantCount("h1") == 2, "Still 2 occupants");

    // Remove
    assertTrue(sys.removeOccupant("h1", "crew1"), "Remove crew1");
    assertTrue(sys.getOccupantCount("h1") == 1, "1 occupant after removal");
    assertTrue(!sys.removeOccupant("h1", "crew1"), "Remove already-removed fails");
}

static void testHangarEnvToxicAtmosphere() {
    std::cout << "\n=== HangarEnvironment: ToxicAtmosphere ===" << std::endl;
    ecs::World world;
    systems::HangarEnvironmentSystem sys(&world);
    world.createEntity("h1");

    using AT = components::HangarEnvironment::AtmosphereType;
    sys.initializeEnvironment("h1", AT::Toxic, 50.0f, 0.5f);
    sys.openHangar("h1");

    assertTrue(sys.getToxicity("h1") == 0.0f, "Toxicity starts at 0");

    // Update to accumulate toxicity (atmosphere_mix_rate=0.1 * delta=5.0 = +0.5)
    sys.update(5.0f);
    assertTrue(sys.getToxicity("h1") > 0.0f, "Toxicity increased after open");
    assertTrue(sys.isAlarmActive("h1"), "Alarm active when toxicity > 0.3");
}

static void testHangarEnvCorrosiveAtmosphere() {
    std::cout << "\n=== HangarEnvironment: CorrosiveAtmosphere ===" << std::endl;
    ecs::World world;
    systems::HangarEnvironmentSystem sys(&world);
    world.createEntity("h1");

    using AT = components::HangarEnvironment::AtmosphereType;
    sys.initializeEnvironment("h1", AT::Corrosive, 30.0f, 0.8f);
    sys.openHangar("h1");

    sys.update(5.0f);
    // Corrosion should accumulate (mix_rate 0.1 * 5.0 = 0.5)
    assertTrue(sys.isAlarmActive("h1"), "Alarm on corrosive exposure");
}

static void testHangarEnvExtremeAtmosphere() {
    std::cout << "\n=== HangarEnvironment: ExtremeAtmosphere ===" << std::endl;
    ecs::World world;
    systems::HangarEnvironmentSystem sys(&world);
    world.createEntity("h1");

    using AT = components::HangarEnvironment::AtmosphereType;
    sys.initializeEnvironment("h1", AT::Extreme, 200.0f, 3.0f);
    sys.openHangar("h1");

    sys.update(5.0f);
    // Extreme: both toxicity AND corrosion
    assertTrue(sys.getToxicity("h1") > 0.0f, "Extreme has toxicity");
    assertTrue(sys.isAlarmActive("h1"), "Alarm active in extreme");
}

static void testHangarEnvBreathableSafe() {
    std::cout << "\n=== HangarEnvironment: BreathableSafe ===" << std::endl;
    ecs::World world;
    systems::HangarEnvironmentSystem sys(&world);
    world.createEntity("h1");

    using AT = components::HangarEnvironment::AtmosphereType;
    sys.initializeEnvironment("h1", AT::Breathable, 22.0f, 1.0f);
    sys.openHangar("h1");

    sys.update(10.0f);
    assertTrue(sys.getToxicity("h1") == 0.0f, "No toxicity in breathable");
    assertTrue(!sys.isAlarmActive("h1"), "No alarm in breathable");
}

static void testHangarEnvOccupantDamage() {
    std::cout << "\n=== HangarEnvironment: OccupantDamage ===" << std::endl;
    ecs::World world;
    systems::HangarEnvironmentSystem sys(&world);
    world.createEntity("h1");

    using AT = components::HangarEnvironment::AtmosphereType;
    sys.initializeEnvironment("h1", AT::Toxic, 50.0f, 0.5f);
    sys.addOccupant("h1", "unsuited", false, 0.0f);
    sys.addOccupant("h1", "suited", true, 0.8f);
    sys.openHangar("h1");

    // Build up toxicity
    sys.update(5.0f);

    float unsuited_dmg = sys.getOccupantDamage("h1", "unsuited");
    float suited_dmg = sys.getOccupantDamage("h1", "suited");

    assertTrue(unsuited_dmg > 0.0f, "Unsuited takes damage");
    assertTrue(suited_dmg > 0.0f, "Suited takes some damage");
    assertTrue(suited_dmg < unsuited_dmg, "Suit reduces damage");
}

static void testHangarEnvRecovery() {
    std::cout << "\n=== HangarEnvironment: Recovery ===" << std::endl;
    ecs::World world;
    systems::HangarEnvironmentSystem sys(&world);
    world.createEntity("h1");

    using AT = components::HangarEnvironment::AtmosphereType;
    sys.initializeEnvironment("h1", AT::Toxic, 50.0f, 0.5f);
    sys.openHangar("h1");

    // Accumulate toxicity
    sys.update(5.0f);
    float toxicity_open = sys.getToxicity("h1");
    assertTrue(toxicity_open > 0.0f, "Toxicity accumulated");

    // Close and recover
    sys.closeHangar("h1");
    sys.update(10.0f);
    float toxicity_closed = sys.getToxicity("h1");
    assertTrue(toxicity_closed < toxicity_open, "Toxicity decreased after closing");
}

static void testHangarEnvSetAtmosphere() {
    std::cout << "\n=== HangarEnvironment: SetAtmosphere ===" << std::endl;
    ecs::World world;
    systems::HangarEnvironmentSystem sys(&world);
    world.createEntity("h1");

    using AT = components::HangarEnvironment::AtmosphereType;
    sys.initializeEnvironment("h1", AT::Breathable, 22.0f, 1.0f);

    assertTrue(sys.setAtmosphere("h1", AT::Toxic), "Change atmosphere succeeds");
    assertTrue(!sys.setAtmosphere("missing", AT::Toxic), "Change on missing fails");
}

static void testHangarEnvMissingEntity() {
    std::cout << "\n=== HangarEnvironment: MissingEntity ===" << std::endl;
    ecs::World world;
    systems::HangarEnvironmentSystem sys(&world);

    assertTrue(sys.getToxicity("x") == 0.0f, "Toxicity default on missing");
    assertTrue(!sys.isAlarmActive("x"), "Alarm default on missing");
    assertTrue(sys.getOccupantCount("x") == 0, "Occupant count on missing");
    assertTrue(sys.getOccupantDamage("x", "o") == 0.0f, "Occupant damage on missing");
    assertTrue(!sys.openHangar("x"), "Open missing fails");
    assertTrue(!sys.closeHangar("x"), "Close missing fails");
    assertTrue(!sys.addOccupant("x", "o", false, 0.0f), "Add occupant on missing fails");
    assertTrue(!sys.removeOccupant("x", "o"), "Remove occupant on missing fails");
}

void run_hangar_environment_system_tests() {
    testHangarEnvInit();
    testHangarEnvInitDuplicate();
    testHangarEnvInitMissing();
    testHangarEnvOpenClose();
    testHangarEnvOccupants();
    testHangarEnvToxicAtmosphere();
    testHangarEnvCorrosiveAtmosphere();
    testHangarEnvExtremeAtmosphere();
    testHangarEnvBreathableSafe();
    testHangarEnvOccupantDamage();
    testHangarEnvRecovery();
    testHangarEnvSetAtmosphere();
    testHangarEnvMissingEntity();
}
