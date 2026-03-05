#ifndef NOVAFORGE_SYSTEMS_CAPTAIN_TRANSFER_SYSTEM_H
#define NOVAFORGE_SYSTEMS_CAPTAIN_TRANSFER_SYSTEM_H

#include "ecs/single_component_system.h"
#include "ecs/entity.h"
#include "components/game_components.h"
#include <string>
#include <vector>

namespace atlas {
namespace systems {

/**
 * @brief Manages captain transfer requests based on morale
 */
class CaptainTransferSystem : public ecs::SingleComponentSystem<components::CaptainTransferRequest> {
public:
    explicit CaptainTransferSystem(ecs::World* world);
    ~CaptainTransferSystem() override = default;

    std::string getName() const override { return "CaptainTransferSystem"; }

    // --- API ---
    bool hasPendingRequest(const std::string& entity_id) const;
    void approveTransfer(const std::string& entity_id);
    void denyTransfer(const std::string& entity_id);
    std::vector<std::string> getPendingTransfers() const;

protected:
    void updateComponent(ecs::Entity& entity, components::CaptainTransferRequest& req, float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_CAPTAIN_TRANSFER_SYSTEM_H
