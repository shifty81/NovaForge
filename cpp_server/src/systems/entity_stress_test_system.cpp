#include "systems/entity_stress_test_system.h"
#include "ecs/world.h"
#include <algorithm>
#include <numeric>
#include <cmath>
#include <memory>

namespace atlas {
namespace systems {

EntityStressTestSystem::EntityStressTestSystem(ecs::World* world)
    : SingleComponentSystem(world) {
}

void EntityStressTestSystem::updateComponent(ecs::Entity& /*entity*/, components::EntityStressTest& test, float /*delta_time*/) {
    // Recompute averages from tick_times
    if (!test.tick_times.empty()) {
        float sum = 0.0f;
        float max_val = 0.0f;
        for (float t : test.tick_times) {
            sum += t;
            max_val = std::max(max_val, t);
        }
        test.avg_tick_ms = sum / static_cast<float>(test.tick_times.size());
        test.max_tick_ms = max_val;
        test.passed_threshold = (test.avg_tick_ms <= test.threshold_ms);
    }
}

bool EntityStressTestSystem::initializeStressTest(const std::string& entity_id,
                                                   const std::string& server_id,
                                                   int target_count) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* existing = entity->getComponent<components::EntityStressTest>();
    if (existing) return false;

    auto comp = std::make_unique<components::EntityStressTest>();
    comp->test_id = entity_id;
    comp->server_id = server_id;
    comp->target_count = target_count;
    comp->stress_phase = components::EntityStressTest::StressPhase::Idle;
    entity->addComponent(std::move(comp));
    return true;
}

bool EntityStressTestSystem::startTest(const std::string& entity_id) {
    auto* test = getComponentFor(entity_id);
    if (!test) return false;

    using SP = components::EntityStressTest::StressPhase;
    if (test->stress_phase != SP::Idle) return false;

    test->stress_phase = SP::Creating;
    test->tick_times.clear();
    test->avg_tick_ms = 0.0f;
    test->max_tick_ms = 0.0f;
    test->queries_per_tick = 0;
    test->avg_query_us = 0.0f;
    test->entity_count = 0;
    test->entity_creation_time_ms = 0.0f;
    test->passed_threshold = false;
    return true;
}

bool EntityStressTestSystem::completeTest(const std::string& entity_id) {
    auto* test = getComponentFor(entity_id);
    if (!test) return false;

    test->stress_phase = components::EntityStressTest::StressPhase::Complete;

    // Final pass computation
    if (!test->tick_times.empty()) {
        float sum = 0.0f;
        float max_val = 0.0f;
        for (float t : test->tick_times) {
            sum += t;
            max_val = std::max(max_val, t);
        }
        test->avg_tick_ms = sum / static_cast<float>(test->tick_times.size());
        test->max_tick_ms = max_val;
    }
    test->passed_threshold = (test->avg_tick_ms <= test->threshold_ms);
    return true;
}

bool EntityStressTestSystem::recordTick(const std::string& entity_id, float tick_time_ms) {
    auto* test = getComponentFor(entity_id);
    if (!test) return false;

    test->tick_times.push_back(tick_time_ms);

    // Recompute stats from full vector for consistency with update()
    float sum = 0.0f;
    float max_val = 0.0f;
    for (float t : test->tick_times) {
        sum += t;
        max_val = std::max(max_val, t);
    }
    test->avg_tick_ms = sum / static_cast<float>(test->tick_times.size());
    test->max_tick_ms = max_val;

    // Transition from Creating to Running once we have entities
    using SP = components::EntityStressTest::StressPhase;
    if (test->stress_phase == SP::Creating && test->entity_count >= test->target_count) {
        test->stress_phase = SP::Running;
    }

    return true;
}

bool EntityStressTestSystem::recordQuery(const std::string& entity_id, float query_time_us) {
    auto* test = getComponentFor(entity_id);
    if (!test) return false;

    test->queries_per_tick++;
    // Running average
    float n = static_cast<float>(test->queries_per_tick);
    test->avg_query_us = test->avg_query_us * ((n - 1.0f) / n) + query_time_us / n;
    return true;
}

bool EntityStressTestSystem::setEntityCount(const std::string& entity_id, int count) {
    auto* test = getComponentFor(entity_id);
    if (!test) return false;

    test->entity_count = count;

    // Transition from Creating to Running once target is reached
    using SP = components::EntityStressTest::StressPhase;
    if (test->stress_phase == SP::Creating && count >= test->target_count) {
        test->stress_phase = SP::Running;
    }
    return true;
}

float EntityStressTestSystem::getAverageTickMs(const std::string& entity_id) const {
    const auto* test = getComponentFor(entity_id);
    if (!test) return 0.0f;

    return test->avg_tick_ms;
}

float EntityStressTestSystem::getMaxTickMs(const std::string& entity_id) const {
    const auto* test = getComponentFor(entity_id);
    if (!test) return 0.0f;

    return test->max_tick_ms;
}

float EntityStressTestSystem::getAverageQueryUs(const std::string& entity_id) const {
    const auto* test = getComponentFor(entity_id);
    if (!test) return 0.0f;

    return test->avg_query_us;
}

bool EntityStressTestSystem::isWithinBudget(const std::string& entity_id) const {
    const auto* test = getComponentFor(entity_id);
    if (!test) return false;

    return test->passed_threshold;
}

std::string EntityStressTestSystem::getPhase(const std::string& entity_id) const {
    const auto* test = getComponentFor(entity_id);
    if (!test) return "";

    using SP = components::EntityStressTest::StressPhase;
    switch (test->stress_phase) {
        case SP::Idle:     return "Idle";
        case SP::Creating: return "Creating";
        case SP::Running:  return "Running";
        case SP::Complete: return "Complete";
    }
    return "";
}

} // namespace systems
} // namespace atlas
