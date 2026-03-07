#ifndef NOVAFORGE_SYSTEMS_CARGO_TRANSFER_SYSTEM_H
#define NOVAFORGE_SYSTEMS_CARGO_TRANSFER_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/economy_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Manages cargo transfers between docked entities
 *
 * Processes active transfer jobs each tick, moving cargo at the configured
 * transfer speed.  Validates that both source and target entities exist.
 */
class CargoTransferSystem : public ecs::SingleComponentSystem<components::CargoTransfer> {
public:
    explicit CargoTransferSystem(ecs::World* world);
    ~CargoTransferSystem() override = default;

    std::string getName() const override { return "CargoTransferSystem"; }

    bool initializeTransfers(const std::string& entity_id, int max_concurrent = 3);
    bool startTransfer(const std::string& entity_id, const std::string& target_id,
                       const std::string& item_type, float amount, float speed = 100.0f);
    int getActiveTransferCount(const std::string& entity_id) const;
    int getTotalCompleted(const std::string& entity_id) const;
    float getTotalUnitsTransferred(const std::string& entity_id) const;
    float getTransferProgress(const std::string& entity_id, int job_index) const;
    bool cancelAllTransfers(const std::string& entity_id);

protected:
    void updateComponent(ecs::Entity& entity, components::CargoTransfer& ct,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_CARGO_TRANSFER_SYSTEM_H
