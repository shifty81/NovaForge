#ifndef NOVAFORGE_SYSTEMS_JITTER_BUFFER_SYSTEM_H
#define NOVAFORGE_SYSTEMS_JITTER_BUFFER_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/navigation_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Network jitter buffer system for smooth client interpolation
 *
 * Tracks packet arrival timing, computes jitter metrics, and maintains
 * an adaptive buffer size to smooth out network variance. Supports
 * underrun/overrun detection and packet loss tracking.
 */
class JitterBufferSystem : public ecs::SingleComponentSystem<components::JitterBuffer> {
public:
    explicit JitterBufferSystem(ecs::World* world);
    ~JitterBufferSystem() override = default;

    std::string getName() const override { return "JitterBufferSystem"; }

public:
    bool initialize(const std::string& entity_id);
    bool recordPacket(const std::string& entity_id, int sequence,
                      float arrival_time, float expected_time);
    bool recordLoss(const std::string& entity_id);
    bool recordUnderrun(const std::string& entity_id);
    bool recordOverrun(const std::string& entity_id);
    bool setBufferBounds(const std::string& entity_id, float min_ms, float max_ms);
    bool setAdaptationRate(const std::string& entity_id, float rate);
    bool resetMetrics(const std::string& entity_id);
    int getSampleCount(const std::string& entity_id) const;
    float getBufferSize(const std::string& entity_id) const;
    float getAverageJitter(const std::string& entity_id) const;
    float getPeakJitter(const std::string& entity_id) const;
    int getTotalPackets(const std::string& entity_id) const;
    int getLostPackets(const std::string& entity_id) const;
    int getUnderruns(const std::string& entity_id) const;
    int getOverruns(const std::string& entity_id) const;
    float getPacketLossRate(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity, components::JitterBuffer& jb, float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_JITTER_BUFFER_SYSTEM_H
