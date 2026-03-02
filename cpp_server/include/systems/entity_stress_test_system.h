#ifndef NOVAFORGE_SYSTEMS_ENTITY_STRESS_TEST_SYSTEM_H
#define NOVAFORGE_SYSTEMS_ENTITY_STRESS_TEST_SYSTEM_H

#include "ecs/system.h"
#include "components/game_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Entity stress test system (Phase 15)
 *
 * Performance validation with 500+ entities. Tracks tick times,
 * query latency, and verifies the system stays within the 20Hz
 * tick budget threshold.
 */
class EntityStressTestSystem : public ecs::System {
public:
    explicit EntityStressTestSystem(ecs::World* world);
    ~EntityStressTestSystem() override = default;

    void update(float delta_time) override;
    std::string getName() const override { return "EntityStressTestSystem"; }

    // Initialization
    bool initializeStressTest(const std::string& entity_id, const std::string& server_id,
                              int target_count);

    // Test control
    bool startTest(const std::string& entity_id);
    bool completeTest(const std::string& entity_id);

    // Recording
    bool recordTick(const std::string& entity_id, float tick_time_ms);
    bool recordQuery(const std::string& entity_id, float query_time_us);
    bool setEntityCount(const std::string& entity_id, int count);

    // Query
    float getAverageTickMs(const std::string& entity_id) const;
    float getMaxTickMs(const std::string& entity_id) const;
    float getAverageQueryUs(const std::string& entity_id) const;
    bool isWithinBudget(const std::string& entity_id) const;
    std::string getPhase(const std::string& entity_id) const;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_ENTITY_STRESS_TEST_SYSTEM_H
