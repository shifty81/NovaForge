#include "systems/client_prediction_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include "components/game_components.h"
#include <cmath>
#include <algorithm>

namespace atlas {
namespace systems {

ClientPredictionSystem::ClientPredictionSystem(ecs::World* world)
    : SingleComponentSystem(world) {
}

static constexpr float PREDICTION_ERROR_THRESHOLD = 0.01f;

void ClientPredictionSystem::updateComponent(ecs::Entity& /*entity*/,
                                              components::ClientPrediction& cp,
                                              float delta_time) {
    if (!cp.active) return;

    // Advance predicted position by velocity
    cp.predicted_x += cp.velocity_x * delta_time;
    cp.predicted_y += cp.velocity_y * delta_time;
    cp.predicted_z += cp.velocity_z * delta_time;

    // Calculate prediction error
    float dx = cp.predicted_x - cp.server_x;
    float dy = cp.predicted_y - cp.server_y;
    float dz = cp.predicted_z - cp.server_z;
    cp.prediction_error = std::sqrt(dx * dx + dy * dy + dz * dz);

    // Blend correction toward server position
    if (cp.prediction_error > PREDICTION_ERROR_THRESHOLD) {
        cp.correction_blend += cp.correction_speed * delta_time;
        cp.correction_blend = std::min(cp.correction_blend, 1.0f);

        // Lerp predicted toward server
        cp.predicted_x = cp.predicted_x + (cp.server_x - cp.predicted_x) * cp.correction_blend;
        cp.predicted_y = cp.predicted_y + (cp.server_y - cp.predicted_y) * cp.correction_blend;
        cp.predicted_z = cp.predicted_z + (cp.server_z - cp.predicted_z) * cp.correction_blend;

        // Snap when blend complete
        if (cp.correction_blend >= 1.0f) {
            cp.predicted_x = cp.server_x;
            cp.predicted_y = cp.server_y;
            cp.predicted_z = cp.server_z;
            cp.correction_blend = 0.0f;
        }
    }
}

bool ClientPredictionSystem::initPrediction(const std::string& entity_id,
                                             const std::string& client_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    if (entity->getComponent<components::ClientPrediction>()) return false;

    auto comp = std::make_unique<components::ClientPrediction>();
    comp->client_id = client_id;
    comp->active = true;
    entity->addComponent(std::move(comp));
    return true;
}

bool ClientPredictionSystem::setServerState(const std::string& entity_id,
                                             float x, float y, float z, int frame) {
    auto* cp = getComponentFor(entity_id);
    if (!cp) return false;

    cp->server_x = x;
    cp->server_y = y;
    cp->server_z = z;
    cp->last_server_frame = frame;
    return true;
}

bool ClientPredictionSystem::applyInput(const std::string& entity_id,
                                         float vx, float vy, float vz, int frame) {
    auto* cp = getComponentFor(entity_id);
    if (!cp) return false;

    cp->velocity_x = vx;
    cp->velocity_y = vy;
    cp->velocity_z = vz;
    cp->prediction_frame = frame;
    return true;
}

float ClientPredictionSystem::getPredictionError(const std::string& entity_id) const {
    auto* cp = getComponentFor(entity_id);
    if (!cp) return 0.0f;

    return cp->prediction_error;
}

bool ClientPredictionSystem::isReconciling(const std::string& entity_id) const {
    auto* cp = getComponentFor(entity_id);
    if (!cp) return false;

    return cp->correction_blend > 0.0f && cp->correction_blend < 1.0f;
}

float ClientPredictionSystem::getCorrectionBlend(const std::string& entity_id) const {
    auto* cp = getComponentFor(entity_id);
    if (!cp) return 0.0f;

    return cp->correction_blend;
}

int ClientPredictionSystem::getPredictionFrame(const std::string& entity_id) const {
    auto* cp = getComponentFor(entity_id);
    if (!cp) return 0;

    return cp->prediction_frame;
}

} // namespace systems
} // namespace atlas
