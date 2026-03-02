#ifndef NOVAFORGE_SYSTEMS_CLIENT_PREDICTION_SYSTEM_H
#define NOVAFORGE_SYSTEMS_CLIENT_PREDICTION_SYSTEM_H

#include "ecs/system.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Server-side client prediction support
 *
 * Manages predicted vs authoritative positions, blending corrections
 * smoothly to provide responsive movement with server reconciliation.
 */
class ClientPredictionSystem : public ecs::System {
public:
    explicit ClientPredictionSystem(ecs::World* world);
    ~ClientPredictionSystem() override = default;

    void update(float delta_time) override;
    std::string getName() const override { return "ClientPredictionSystem"; }

    bool initPrediction(const std::string& entity_id, const std::string& client_id);
    bool setServerState(const std::string& entity_id, float x, float y, float z, int frame);
    bool applyInput(const std::string& entity_id, float vx, float vy, float vz, int frame);
    float getPredictionError(const std::string& entity_id) const;
    bool isReconciling(const std::string& entity_id) const;
    float getCorrectionBlend(const std::string& entity_id) const;
    int getPredictionFrame(const std::string& entity_id) const;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_CLIENT_PREDICTION_SYSTEM_H
