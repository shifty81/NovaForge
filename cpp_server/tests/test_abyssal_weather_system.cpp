// Tests for: AbyssalWeatherSystem
#include "test_log.h"
#include "components/core_components.h"
#include "components/exploration_components.h"
#include "ecs/system.h"
#include "systems/abyssal_weather_system.h"

using namespace atlas;
using WT = components::AbyssalWeatherState::WeatherType;

// ==================== AbyssalWeatherSystem Tests ====================

static void testAbyssalWeatherCreate() {
    std::cout << "\n=== AbyssalWeather: Create ===" << std::endl;
    ecs::World world;
    systems::AbyssalWeatherSystem sys(&world);
    world.createEntity("pocket1");
    assertTrue(sys.initialize("pocket1", "abyssal_pocket_01"), "Init succeeds");
    assertTrue(sys.getWeather("pocket1") == WT::None, "No weather initially");
    assertTrue(approxEqual(sys.getIntensity("pocket1"), 0.0f), "0 intensity initially");
    assertTrue(approxEqual(sys.getTurretOptimalModifier("pocket1"), 1.0f), "Default turret mod");
    assertTrue(approxEqual(sys.getMissileVelocityModifier("pocket1"), 1.0f), "Default missile mod");
    assertTrue(approxEqual(sys.getDroneSpeedModifier("pocket1"), 1.0f), "Default drone mod");
    assertTrue(approxEqual(sys.getShieldHpModifier("pocket1"), 1.0f), "Default shield mod");
    assertTrue(approxEqual(sys.getArmorHpModifier("pocket1"), 1.0f), "Default armor mod");
    assertTrue(approxEqual(sys.getCapacitorRechargeModifier("pocket1"), 1.0f), "Default cap mod");
    assertTrue(approxEqual(sys.getPropulsionModifier("pocket1"), 1.0f), "Default prop mod");
    assertTrue(approxEqual(sys.getEwStrengthModifier("pocket1"), 1.0f), "Default EW mod");
}

static void testAbyssalWeatherElectrical() {
    std::cout << "\n=== AbyssalWeather: Electrical ===" << std::endl;
    ecs::World world;
    systems::AbyssalWeatherSystem sys(&world);
    world.createEntity("pocket1");
    sys.initialize("pocket1");

    assertTrue(sys.setWeather("pocket1", WT::Electrical, 1), "Set Electrical T1");
    assertTrue(sys.getWeather("pocket1") == WT::Electrical, "Electrical weather");
    assertTrue(approxEqual(sys.getIntensity("pocket1"), 1.0f), "Intensity 1.0");
    // At T1 intensity=1.0: boost = 1.0 + 0*0.2 = 1.0, debuff = 1.0 - 0*0.1 = 1.0
    assertTrue(approxEqual(sys.getTurretOptimalModifier("pocket1"), 1.0f),
               "T1 turret mod = 1.0");
    assertTrue(approxEqual(sys.getCapacitorRechargeModifier("pocket1"), 1.0f),
               "T1 cap mod = 1.0");
}

static void testAbyssalWeatherElectricalT3() {
    std::cout << "\n=== AbyssalWeather: ElectricalT3 ===" << std::endl;
    ecs::World world;
    systems::AbyssalWeatherSystem sys(&world);
    world.createEntity("pocket1");
    sys.initialize("pocket1");

    sys.setWeather("pocket1", WT::Electrical, 3);
    assertTrue(approxEqual(sys.getIntensity("pocket1"), 3.0f), "Intensity 3.0");
    // At T3 intensity=3.0: boost = 1.0 + 2*0.2 = 1.4, debuff = 1.0 - 2*0.1 = 0.8
    assertTrue(approxEqual(sys.getTurretOptimalModifier("pocket1"), 1.4f),
               "T3 turret optimal boost = 1.4");
    assertTrue(approxEqual(sys.getCapacitorRechargeModifier("pocket1"), 0.8f),
               "T3 cap recharge debuff = 0.8");
    // Other modifiers unchanged
    assertTrue(approxEqual(sys.getMissileVelocityModifier("pocket1"), 1.0f),
               "Missile unchanged for Electrical");
}

static void testAbyssalWeatherDarkMatter() {
    std::cout << "\n=== AbyssalWeather: DarkMatter ===" << std::endl;
    ecs::World world;
    systems::AbyssalWeatherSystem sys(&world);
    world.createEntity("pocket1");
    sys.initialize("pocket1");

    sys.setWeather("pocket1", WT::DarkMatter, 5);
    // At T5 intensity=5.0: boost = 1.8, debuff = 0.6
    assertTrue(approxEqual(sys.getDroneSpeedModifier("pocket1"), 0.6f),
               "T5 dark matter drone speed = 0.6");
    // Check turret unchanged
    assertTrue(approxEqual(sys.getTurretOptimalModifier("pocket1"), 1.0f),
               "Turret unchanged for DarkMatter");
}

static void testAbyssalWeatherExoticPlasma() {
    std::cout << "\n=== AbyssalWeather: ExoticPlasma ===" << std::endl;
    ecs::World world;
    systems::AbyssalWeatherSystem sys(&world);
    world.createEntity("pocket1");
    sys.initialize("pocket1");

    sys.setWeather("pocket1", WT::ExoticPlasma, 2);
    // At T2 intensity=2.0: boost = 1.2, debuff = 0.9
    assertTrue(approxEqual(sys.getMissileVelocityModifier("pocket1"), 1.2f),
               "T2 exotic plasma missile boost = 1.2");
    assertTrue(approxEqual(sys.getArmorHpModifier("pocket1"), 0.9f),
               "T2 exotic plasma armor debuff = 0.9");
}

static void testAbyssalWeatherGamma() {
    std::cout << "\n=== AbyssalWeather: Gamma ===" << std::endl;
    ecs::World world;
    systems::AbyssalWeatherSystem sys(&world);
    world.createEntity("pocket1");
    sys.initialize("pocket1");

    sys.setWeather("pocket1", WT::Gamma, 4);
    // At T4 intensity=4.0: boost = 1.6, debuff = 0.7
    assertTrue(approxEqual(sys.getShieldHpModifier("pocket1"), 1.6f),
               "T4 gamma shield boost = 1.6");
    // hull_hp uses debuff formula
    // debuff = 1.0 - (4-1)*0.1 = 0.7
    assertTrue(approxEqual(sys.getCapacitorRechargeModifier("pocket1"), 1.0f),
               "Cap unchanged for Gamma");
}

static void testAbyssalWeatherFirestorm() {
    std::cout << "\n=== AbyssalWeather: Firestorm ===" << std::endl;
    ecs::World world;
    systems::AbyssalWeatherSystem sys(&world);
    world.createEntity("pocket1");
    sys.initialize("pocket1");

    sys.setWeather("pocket1", WT::Firestorm, 2);
    // At T2 intensity=2.0: boost=1.2, debuff=0.9
    assertTrue(approxEqual(sys.getEwStrengthModifier("pocket1"), 1.2f),
               "T2 firestorm EW boost = 1.2");
    assertTrue(approxEqual(sys.getPropulsionModifier("pocket1"), 0.9f),
               "T2 firestorm propulsion debuff = 0.9");
}

static void testAbyssalWeatherTierClamping() {
    std::cout << "\n=== AbyssalWeather: TierClamping ===" << std::endl;
    ecs::World world;
    systems::AbyssalWeatherSystem sys(&world);
    world.createEntity("pocket1");
    sys.initialize("pocket1");

    // Tier 0 clamped to 1
    sys.setWeather("pocket1", WT::Electrical, 0);
    assertTrue(approxEqual(sys.getIntensity("pocket1"), 1.0f), "Tier 0 → intensity 1.0");

    // Tier 10 clamped to 5
    sys.setWeather("pocket1", WT::Electrical, 10);
    assertTrue(approxEqual(sys.getIntensity("pocket1"), 5.0f), "Tier 10 → intensity 5.0");
}

static void testAbyssalWeatherMissing() {
    std::cout << "\n=== AbyssalWeather: Missing ===" << std::endl;
    ecs::World world;
    systems::AbyssalWeatherSystem sys(&world);
    assertTrue(!sys.initialize("nonexistent"), "Init fails on missing entity");
    assertTrue(!sys.setWeather("nonexistent", WT::Electrical, 1),
               "SetWeather fails on missing");
    assertTrue(sys.getWeather("nonexistent") == WT::None, "None on missing");
    assertTrue(approxEqual(sys.getIntensity("nonexistent"), 0.0f), "0 on missing");
    assertTrue(approxEqual(sys.getTurretOptimalModifier("nonexistent"), 1.0f),
               "1.0 default on missing");
}

void run_abyssal_weather_system_tests() {
    testAbyssalWeatherCreate();
    testAbyssalWeatherElectrical();
    testAbyssalWeatherElectricalT3();
    testAbyssalWeatherDarkMatter();
    testAbyssalWeatherExoticPlasma();
    testAbyssalWeatherGamma();
    testAbyssalWeatherFirestorm();
    testAbyssalWeatherTierClamping();
    testAbyssalWeatherMissing();
}
