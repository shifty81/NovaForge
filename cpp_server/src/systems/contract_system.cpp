#include "systems/contract_system.h"
#include "ecs/world.h"
#include "components/game_components.h"

namespace atlas {
namespace systems {

ContractSystem::ContractSystem(ecs::World* world)
    : SingleComponentSystem(world) {
}

void ContractSystem::updateComponent(ecs::Entity& entity, components::ContractBoard& board, float delta_time) {
    for (auto& contract : board.contracts) {
        if (contract.status != "outstanding") continue;
        if (contract.duration_remaining > 0.0f) {
            contract.duration_remaining -= delta_time;
            if (contract.duration_remaining <= 0.0f) {
                contract.duration_remaining = 0.0f;
                contract.status = "expired";
            }
        }
    }
}

bool ContractSystem::createContract(const std::string& board_entity_id,
                                    const std::string& issuer_id,
                                    const std::string& type,
                                    double isc_reward,
                                    float duration_seconds) {
    auto* board = getComponentFor(board_entity_id);
    if (!board) return false;

    components::ContractBoard::Contract contract;
    contract.contract_id = "contract_" + issuer_id + "_" + std::to_string(board->contracts.size());
    contract.issuer_id = issuer_id;
    contract.type = type;
    contract.isc_reward = isc_reward;
    contract.duration_remaining = duration_seconds;
    contract.status = "outstanding";

    board->contracts.push_back(contract);
    return true;
}

bool ContractSystem::acceptContract(const std::string& board_entity_id,
                                    const std::string& contract_id,
                                    const std::string& acceptor_id) {
    auto* board = getComponentFor(board_entity_id);
    if (!board) return false;

    for (auto& contract : board->contracts) {
        if (contract.contract_id == contract_id) {
            if (contract.status != "outstanding") return false;
            contract.assignee_id = acceptor_id;
            contract.status = "in_progress";
            return true;
        }
    }
    return false;
}

bool ContractSystem::completeContract(const std::string& board_entity_id,
                                      const std::string& contract_id) {
    auto* board = getComponentFor(board_entity_id);
    if (!board) return false;

    for (auto& contract : board->contracts) {
        if (contract.contract_id == contract_id) {
            if (contract.status != "in_progress") return false;
            contract.status = "completed";

            if (!contract.assignee_id.empty()) {
                auto* assignee = world_->getEntity(contract.assignee_id);
                if (assignee) {
                    auto* player = assignee->getComponent<components::Player>();
                    if (player) {
                        player->credits += contract.isc_reward;
                    }
                }
            }
            return true;
        }
    }
    return false;
}

int ContractSystem::getActiveContractCount(const std::string& board_entity_id) {
    auto* board = getComponentFor(board_entity_id);
    if (!board) return 0;

    int count = 0;
    for (const auto& contract : board->contracts) {
        if (contract.status == "outstanding" || contract.status == "in_progress")
            ++count;
    }
    return count;
}

int ContractSystem::getContractsByStatus(const std::string& board_entity_id,
                                         const std::string& status) {
    auto* board = getComponentFor(board_entity_id);
    if (!board) return 0;

    int count = 0;
    for (const auto& contract : board->contracts) {
        if (contract.status == status)
            ++count;
    }
    return count;
}

} // namespace systems
} // namespace atlas
