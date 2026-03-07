#include "systems/contract_board_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>

namespace atlas {
namespace systems {

ContractBoardSystem::ContractBoardSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void ContractBoardSystem::updateComponent(ecs::Entity& entity,
    components::ContractBoard& comp, float delta_time) {
    // Tick down outstanding and in_progress contract durations
    for (auto& c : comp.contracts) {
        if (c.status == "outstanding" || c.status == "in_progress") {
            if (c.duration_remaining > 0.0f) {
                c.duration_remaining -= delta_time;
                if (c.duration_remaining <= 0.0f) {
                    c.duration_remaining = 0.0f;
                    c.status = "expired";
                }
            }
        }
    }
}

bool ContractBoardSystem::initialize(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::ContractBoard>();
    entity->addComponent(std::move(comp));
    return true;
}

bool ContractBoardSystem::postContract(const std::string& entity_id,
    const std::string& contract_id, const std::string& issuer_id,
    const std::string& type, double isc_reward, float days_to_complete) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;

    // Prevent duplicate contract IDs
    for (const auto& c : comp->contracts) {
        if (c.contract_id == contract_id) return false;
    }

    components::ContractBoard::Contract contract;
    contract.contract_id = contract_id;
    contract.issuer_id = issuer_id;
    contract.type = type;
    contract.isc_reward = isc_reward;
    contract.days_to_complete = days_to_complete;
    contract.duration_remaining = days_to_complete * 86400.0f; // days → seconds
    contract.status = "outstanding";
    comp->contracts.push_back(contract);
    return true;
}

bool ContractBoardSystem::acceptContract(const std::string& entity_id,
    const std::string& contract_id, const std::string& assignee_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (auto& c : comp->contracts) {
        if (c.contract_id == contract_id && c.status == "outstanding") {
            c.assignee_id = assignee_id;
            c.status = "in_progress";
            return true;
        }
    }
    return false;
}

bool ContractBoardSystem::completeContract(const std::string& entity_id,
    const std::string& contract_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (auto& c : comp->contracts) {
        if (c.contract_id == contract_id && c.status == "in_progress") {
            c.status = "completed";
            return true;
        }
    }
    return false;
}

bool ContractBoardSystem::failContract(const std::string& entity_id,
    const std::string& contract_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (auto& c : comp->contracts) {
        if (c.contract_id == contract_id && c.status == "in_progress") {
            c.status = "failed";
            return true;
        }
    }
    return false;
}

int ContractBoardSystem::getContractCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? static_cast<int>(comp->contracts.size()) : 0;
}

int ContractBoardSystem::getOutstandingCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    return static_cast<int>(std::count_if(comp->contracts.begin(), comp->contracts.end(),
        [](const components::ContractBoard::Contract& c) { return c.status == "outstanding"; }));
}

int ContractBoardSystem::getCompletedCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    return static_cast<int>(std::count_if(comp->contracts.begin(), comp->contracts.end(),
        [](const components::ContractBoard::Contract& c) { return c.status == "completed"; }));
}

int ContractBoardSystem::getExpiredCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    return static_cast<int>(std::count_if(comp->contracts.begin(), comp->contracts.end(),
        [](const components::ContractBoard::Contract& c) { return c.status == "expired"; }));
}

std::string ContractBoardSystem::getContractStatus(const std::string& entity_id,
    const std::string& contract_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return "unknown";
    for (const auto& c : comp->contracts) {
        if (c.contract_id == contract_id) return c.status;
    }
    return "unknown";
}

} // namespace systems
} // namespace atlas
