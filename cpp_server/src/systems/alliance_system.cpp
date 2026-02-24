#include "systems/alliance_system.h"
#include "ecs/world.h"
#include "components/game_components.h"
#include <algorithm>
#include <cctype>

namespace atlas {
namespace systems {

AllianceSystem::AllianceSystem(ecs::World* world)
    : System(world) {
}

void AllianceSystem::update(float /*delta_time*/) {
}

bool AllianceSystem::createAlliance(const std::string& corp_entity_id,
                                    const std::string& alliance_name,
                                    const std::string& ticker) {
    auto* corp_entity = world_->getEntity(corp_entity_id);
    if (!corp_entity) return false;

    auto* corp = corp_entity->getComponent<components::Corporation>();
    if (!corp) return false;

    // Build alliance entity ID: "alliance_" + lowercased name with spaces replaced by underscores
    std::string alliance_id = "alliance_";
    for (char c : alliance_name) {
        if (c == ' ')
            alliance_id += '_';
        else if (std::isalnum(static_cast<unsigned char>(c)))
            alliance_id += static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        else
            return false; // Reject names with special characters
    }

    auto* alliance_entity = world_->createEntity(alliance_id);
    if (!alliance_entity) return false;

    auto alliance = std::make_unique<components::Alliance>();
    alliance->alliance_id = alliance_id;
    alliance->alliance_name = alliance_name;
    alliance->ticker = ticker;
    alliance->executor_corp_id = corp_entity_id;
    alliance->member_corp_ids.push_back(corp_entity_id);
    alliance_entity->addComponent(std::move(alliance));

    return true;
}

bool AllianceSystem::joinAlliance(const std::string& corp_entity_id,
                                  const std::string& alliance_entity_id) {
    auto* corp_entity = world_->getEntity(corp_entity_id);
    if (!corp_entity) return false;

    auto* corp = corp_entity->getComponent<components::Corporation>();
    if (!corp) return false;

    auto* alliance_entity = world_->getEntity(alliance_entity_id);
    if (!alliance_entity) return false;

    auto* alliance = alliance_entity->getComponent<components::Alliance>();
    if (!alliance) return false;

    // Check for duplicates
    for (const auto& id : alliance->member_corp_ids) {
        if (id == corp_entity_id) return false;
    }

    // Check max corps
    if (static_cast<int>(alliance->member_corp_ids.size()) >= alliance->max_corps) return false;

    alliance->member_corp_ids.push_back(corp_entity_id);
    return true;
}

bool AllianceSystem::leaveAlliance(const std::string& corp_entity_id,
                                   const std::string& alliance_entity_id) {
    auto* alliance_entity = world_->getEntity(alliance_entity_id);
    if (!alliance_entity) return false;

    auto* alliance = alliance_entity->getComponent<components::Alliance>();
    if (!alliance) return false;

    // Executor corp cannot leave
    if (alliance->executor_corp_id == corp_entity_id) return false;

    auto it = std::find(alliance->member_corp_ids.begin(),
                        alliance->member_corp_ids.end(),
                        corp_entity_id);
    if (it == alliance->member_corp_ids.end()) return false;

    alliance->member_corp_ids.erase(it);
    return true;
}

bool AllianceSystem::setExecutor(const std::string& alliance_entity_id,
                                 const std::string& new_executor_corp_id,
                                 const std::string& requester_corp_id) {
    auto* alliance_entity = world_->getEntity(alliance_entity_id);
    if (!alliance_entity) return false;

    auto* alliance = alliance_entity->getComponent<components::Alliance>();
    if (!alliance) return false;

    // Only current executor can change executor
    if (alliance->executor_corp_id != requester_corp_id) return false;

    // New executor must be a member
    bool found = false;
    for (const auto& id : alliance->member_corp_ids) {
        if (id == new_executor_corp_id) { found = true; break; }
    }
    if (!found) return false;

    alliance->executor_corp_id = new_executor_corp_id;
    return true;
}

int AllianceSystem::getMemberCorpCount(const std::string& alliance_entity_id) {
    auto* alliance_entity = world_->getEntity(alliance_entity_id);
    if (!alliance_entity) return 0;

    auto* alliance = alliance_entity->getComponent<components::Alliance>();
    if (!alliance) return 0;

    return static_cast<int>(alliance->member_corp_ids.size());
}

bool AllianceSystem::isCorpInAlliance(const std::string& corp_entity_id,
                                      const std::string& alliance_entity_id) {
    auto* alliance_entity = world_->getEntity(alliance_entity_id);
    if (!alliance_entity) return false;

    auto* alliance = alliance_entity->getComponent<components::Alliance>();
    if (!alliance) return false;

    for (const auto& id : alliance->member_corp_ids) {
        if (id == corp_entity_id) return true;
    }
    return false;
}

bool AllianceSystem::disbandAlliance(const std::string& alliance_entity_id,
                                     const std::string& requester_corp_id) {
    auto* alliance_entity = world_->getEntity(alliance_entity_id);
    if (!alliance_entity) return false;

    auto* alliance = alliance_entity->getComponent<components::Alliance>();
    if (!alliance) return false;

    // Only executor can disband
    if (alliance->executor_corp_id != requester_corp_id) return false;

    world_->destroyEntity(alliance_entity_id);
    return true;
}

} // namespace systems
} // namespace atlas
