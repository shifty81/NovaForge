#ifndef EVE_SYSTEMS_CAPTAIN_TRANSFER_SYSTEM_H
#define EVE_SYSTEMS_CAPTAIN_TRANSFER_SYSTEM_H

#include "ecs/system.h"
#include "ecs/entity.h"
#include "components/game_components.h"
#include <string>
#include <vector>

namespace atlas {
namespace systems {

/**
 * @brief Manages captain transfer requests based on morale
 */
class CaptainTransferSystem : public ecs::System {
public:
    explicit CaptainTransferSystem(ecs::World* world);
    ~CaptainTransferSystem() override = default;

    void update(float delta_time) override;
    std::string getName() const override { return "CaptainTransferSystem"; }

    // --- API ---
    bool hasPendingRequest(const std::string& entity_id) const;
    void approveTransfer(const std::string& entity_id);
    void denyTransfer(const std::string& entity_id);
    std::vector<std::string> getPendingTransfers() const;
};

} // namespace systems
} // namespace atlas

#endif // EVE_SYSTEMS_CAPTAIN_TRANSFER_SYSTEM_H
