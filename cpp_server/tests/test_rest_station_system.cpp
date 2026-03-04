// Tests for: Rest Station System Tests
#include "test_log.h"
#include "components/crew_components.h"
#include "components/ship_components.h"
#include "ecs/system.h"
#include "systems/rest_station_system.h"
#include "systems/station_system.h"
#include <sys/stat.h>

using namespace atlas;

// ==================== Rest Station System Tests ====================

static void testRestStationDefaults() {
    std::cout << "\n=== Rest Station Defaults ===" << std::endl;
    ecs::World world;
    auto* station = world.createEntity("bed1");
    auto* rs = addComp<components::RestStation>(station);
    
    assertTrue(rs->isAvailable(), "New station is available");
    assertTrue(approxEqual(rs->quality, 1.0f), "Default quality is 1.0");
    assertTrue(rs->type == components::RestStation::StationType::Bed, "Default type is Bed");
}

static void testRestStationStartRest() {
    std::cout << "\n=== Rest Station Start Rest ===" << std::endl;
    ecs::World world;
    systems::RestStationSystem restSys(&world);
    
    auto* character = world.createEntity("char1");
    auto* needs = addComp<components::SurvivalNeeds>(character);
    needs->fatigue = 80.0f;
    
    auto* station = world.createEntity("bed2");
    addComp<components::RestStation>(station);
    
    bool started = restSys.startResting("char1", "bed2");
    assertTrue(started, "Rest started successfully");
    assertTrue(restSys.isResting("char1"), "Character is resting");
    assertTrue(!restSys.isStationAvailable("bed2"), "Station is now occupied");
}

static void testRestStationRecovery() {
    std::cout << "\n=== Rest Station Recovery ===" << std::endl;
    ecs::World world;
    systems::RestStationSystem restSys(&world);
    
    auto* character = world.createEntity("char2");
    auto* needs = addComp<components::SurvivalNeeds>(character);
    needs->fatigue = 50.0f;
    
    auto* station = world.createEntity("bed3");
    auto* rs = addComp<components::RestStation>(station);
    rs->quality = 2.0f;  // Luxury
    
    restSys.startResting("char2", "bed3");
    restSys.update(10.0f);  // 10 seconds of rest
    
    // Recovery = base_rate * quality * time = 1.0 * 2.0 * 10 = 20
    assertTrue(approxEqual(needs->fatigue, 30.0f), "Fatigue reduced by 20 (50 - 20 = 30)");
}

static void testRestStationStopRest() {
    std::cout << "\n=== Rest Station Stop Rest ===" << std::endl;
    ecs::World world;
    systems::RestStationSystem restSys(&world);
    
    auto* character = world.createEntity("char3");
    auto* needs = addComp<components::SurvivalNeeds>(character);
    needs->fatigue = 40.0f;
    
    auto* station = world.createEntity("bed4");
    addComp<components::RestStation>(station);
    
    restSys.startResting("char3", "bed4");
    restSys.update(5.0f);
    
    float finalFatigue = restSys.stopResting("char3");
    assertTrue(finalFatigue < 40.0f, "Fatigue reduced after rest");
    assertTrue(!restSys.isResting("char3"), "Character no longer resting");
    assertTrue(restSys.isStationAvailable("bed4"), "Station available again");
}

static void testRestStationProgress() {
    std::cout << "\n=== Rest Station Progress ===" << std::endl;
    ecs::World world;
    systems::RestStationSystem restSys(&world);
    
    auto* character = world.createEntity("char4");
    auto* needs = addComp<components::SurvivalNeeds>(character);
    needs->fatigue = 100.0f;
    
    auto* station = world.createEntity("bed5");
    addComp<components::RestStation>(station);
    
    restSys.startResting("char4", "bed5");
    restSys.update(50.0f);  // Should recover 50 fatigue points
    
    float progress = restSys.getRestProgress("char4");
    assertTrue(approxEqual(progress, 0.5f), "50% rest progress (50/100)");
}

static void testRestStationQualityName() {
    std::cout << "\n=== Rest Station Quality Name ===" << std::endl;
    assertTrue(systems::RestStationSystem::getQualityName(2.0f) == "Luxury", "Quality 2.0 is Luxury");
    assertTrue(systems::RestStationSystem::getQualityName(1.5f) == "Premium", "Quality 1.5 is Premium");
    assertTrue(systems::RestStationSystem::getQualityName(1.0f) == "Standard", "Quality 1.0 is Standard");
    assertTrue(systems::RestStationSystem::getQualityName(0.5f) == "Basic", "Quality 0.5 is Basic");
    assertTrue(systems::RestStationSystem::getQualityName(0.3f) == "Poor", "Quality 0.3 is Poor");
}

static void testRestStationAutoStop() {
    std::cout << "\n=== Rest Station Auto Stop ===" << std::endl;
    ecs::World world;
    systems::RestStationSystem restSys(&world);
    
    auto* character = world.createEntity("char5");
    auto* needs = addComp<components::SurvivalNeeds>(character);
    needs->fatigue = 10.0f;
    
    auto* station = world.createEntity("bed6");
    addComp<components::RestStation>(station);
    
    restSys.startResting("char5", "bed6");
    restSys.update(15.0f);  // Should fully recover and auto-stop
    
    assertTrue(!restSys.isResting("char5"), "Character auto-stopped resting when fully rested");
    assertTrue(needs->fatigue <= 0.0f, "Fatigue at 0 when fully rested");
}

static void testRestStationCount() {
    std::cout << "\n=== Rest Station Count ===" << std::endl;
    ecs::World world;
    systems::RestStationSystem restSys(&world);
    
    auto* char1 = world.createEntity("c1");
    addComp<components::SurvivalNeeds>(char1)->fatigue = 50.0f;
    auto* char2 = world.createEntity("c2");
    addComp<components::SurvivalNeeds>(char2)->fatigue = 50.0f;
    
    auto* bed1 = world.createEntity("b1");
    addComp<components::RestStation>(bed1);
    auto* bed2 = world.createEntity("b2");
    addComp<components::RestStation>(bed2);
    
    assertTrue(restSys.getRestingCount() == 0, "No one resting initially");
    
    restSys.startResting("c1", "b1");
    assertTrue(restSys.getRestingCount() == 1, "1 person resting");
    
    restSys.startResting("c2", "b2");
    assertTrue(restSys.getRestingCount() == 2, "2 people resting");
}


void run_rest_station_system_tests() {
    testRestStationDefaults();
    testRestStationStartRest();
    testRestStationRecovery();
    testRestStationStopRest();
    testRestStationProgress();
    testRestStationQualityName();
    testRestStationAutoStop();
    testRestStationCount();
}
