#ifndef NOVAFORGE_SYSTEMS_CONTRACT_SYSTEM_H
#define NOVAFORGE_SYSTEMS_CONTRACT_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/game_components.h"
#include <string>

namespace atlas {
namespace systems {

class ContractSystem : public ecs::SingleComponentSystem<components::ContractBoard> {
public:
    explicit ContractSystem(ecs::World* world);
    ~ContractSystem() override = default;

    std::string getName() const override { return "ContractSystem"; }

    bool createContract(const std::string& board_entity_id,
                        const std::string& issuer_id,
                        const std::string& type,
                        double isk_reward,
                        float duration_seconds);

    bool acceptContract(const std::string& board_entity_id,
                        const std::string& contract_id,
                        const std::string& acceptor_id);

    bool completeContract(const std::string& board_entity_id,
                          const std::string& contract_id);

    int getActiveContractCount(const std::string& board_entity_id);

    int getContractsByStatus(const std::string& board_entity_id,
                             const std::string& status);

protected:
    void updateComponent(ecs::Entity& entity, components::ContractBoard& comp, float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_CONTRACT_SYSTEM_H
