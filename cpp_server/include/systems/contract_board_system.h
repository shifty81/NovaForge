#ifndef NOVAFORGE_SYSTEMS_CONTRACT_BOARD_SYSTEM_H
#define NOVAFORGE_SYSTEMS_CONTRACT_BOARD_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/economy_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Station contract board — list, accept, complete, expire contracts
 *
 * Each station entity owns a ContractBoard component.  Contracts have a
 * duration timer that ticks down each update.  Players accept contracts
 * which move to "in_progress", then complete or fail them.
 */
class ContractBoardSystem : public ecs::SingleComponentSystem<components::ContractBoard> {
public:
    explicit ContractBoardSystem(ecs::World* world);
    ~ContractBoardSystem() override = default;

    std::string getName() const override { return "ContractBoardSystem"; }

public:
    bool initialize(const std::string& entity_id);
    bool postContract(const std::string& entity_id, const std::string& contract_id,
                      const std::string& issuer_id, const std::string& type,
                      double isc_reward, float days_to_complete);
    bool acceptContract(const std::string& entity_id, const std::string& contract_id,
                        const std::string& assignee_id);
    bool completeContract(const std::string& entity_id, const std::string& contract_id);
    bool failContract(const std::string& entity_id, const std::string& contract_id);

    int getContractCount(const std::string& entity_id) const;
    int getOutstandingCount(const std::string& entity_id) const;
    int getCompletedCount(const std::string& entity_id) const;
    int getExpiredCount(const std::string& entity_id) const;
    std::string getContractStatus(const std::string& entity_id,
                                  const std::string& contract_id) const;

protected:
    void updateComponent(ecs::Entity& entity, components::ContractBoard& comp,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_CONTRACT_BOARD_SYSTEM_H
