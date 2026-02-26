#include "systems/myth_boss_system.h"
#include "ecs/world.h"
#include <algorithm>
#include <memory>

namespace atlas {
namespace systems {

MythBossSystem::MythBossSystem(ecs::World* world)
    : System(world) {
}

void MythBossSystem::update(float delta_time) {
    auto entities = world_->getEntities<components::MythBossEncounter>();
    for (auto* entity : entities) {
        auto* enc = entity->getComponent<components::MythBossEncounter>();
        if (!enc || !enc->active) continue;

        enc->active_time += delta_time;

        if (enc->isExpired()) {
            enc->active = false;
        }
    }
}

std::string MythBossSystem::generateEncounter(const std::string& myth_id,
                                               const std::string& system_id) {
    // Look up the myth from the propaganda network
    components::PropagandaNetwork::MythEntry* myth = nullptr;
    auto networks = world_->getEntities<components::PropagandaNetwork>();
    for (auto* entity : networks) {
        auto* network = entity->getComponent<components::PropagandaNetwork>();
        if (!network) continue;
        myth = network->findMyth(myth_id);
        if (myth) break;
    }

    // Create encounter entity
    std::string enc_id = "encounter_" + std::to_string(++encounter_counter_);
    auto* entity = world_->createEntity(enc_id);
    auto comp = std::make_unique<components::MythBossEncounter>();

    comp->encounter_id = enc_id;
    comp->myth_id = myth_id;
    comp->system_id = system_id;

    if (myth) {
        // Map myth type to boss type
        switch (myth->type) {
            case components::PropagandaNetwork::MythType::Heroic:
                comp->boss_type = components::MythBossEncounter::BossType::Guardian;
                break;
            case components::PropagandaNetwork::MythType::Villainous:
                comp->boss_type = components::MythBossEncounter::BossType::Destroyer;
                break;
            case components::PropagandaNetwork::MythType::Mysterious:
                comp->boss_type = components::MythBossEncounter::BossType::Phantom;
                break;
            case components::PropagandaNetwork::MythType::Exaggerated:
                comp->boss_type = components::MythBossEncounter::BossType::Colossus;
                break;
            case components::PropagandaNetwork::MythType::Fabricated:
                comp->boss_type = components::MythBossEncounter::BossType::Mirage;
                break;
        }

        // Calculate difficulty from spread_count and credibility
        float spread_factor = std::min(static_cast<float>(myth->spread_count), 10.0f) / 2.0f;
        comp->difficulty = std::max(1.0f, spread_factor * myth->credibility);

        // Scale stats by difficulty
        comp->shield_hp = 1000.0f * comp->difficulty;
        comp->armor_hp = 500.0f * comp->difficulty;
        comp->hull_hp = 2000.0f * comp->difficulty;
        comp->recommended_fleet_size = std::max(3, static_cast<int>(comp->difficulty * 2.0f));

        // Generate loot table based on difficulty
        int loot_count = std::max(1, static_cast<int>(comp->difficulty));
        for (int i = 0; i < loot_count; ++i) {
            components::MythBossEncounter::LootEntry entry;
            entry.item_id = "myth_loot_" + std::to_string(i + 1);
            entry.drop_chance = std::min(1.0f, 0.3f + comp->difficulty * 0.1f);
            entry.quantity = 1 + static_cast<int>(comp->difficulty * 0.5f);
            comp->loot_table.push_back(entry);
        }
    }

    comp->active = true;
    comp->active_time = 0.0f;

    entity->addComponent(std::move(comp));
    return enc_id;
}

bool MythBossSystem::isEncounterActive(const std::string& encounter_id) const {
    auto* entity = world_->getEntity(encounter_id);
    if (!entity) return false;

    auto* enc = entity->getComponent<components::MythBossEncounter>();
    return enc && enc->isActive();
}

bool MythBossSystem::completeEncounter(const std::string& encounter_id, bool success) {
    auto* entity = world_->getEntity(encounter_id);
    if (!entity) return false;

    auto* enc = entity->getComponent<components::MythBossEncounter>();
    if (!enc) return false;

    enc->active = false;
    enc->completion_success = success;
    return true;
}

int MythBossSystem::getActiveBossCount() const {
    int count = 0;
    auto entities = world_->getEntities<components::MythBossEncounter>();
    for (auto* entity : entities) {
        auto* enc = entity->getComponent<components::MythBossEncounter>();
        if (enc && enc->isActive()) count++;
    }
    return count;
}

float MythBossSystem::getBossDifficulty(const std::string& encounter_id) const {
    auto* entity = world_->getEntity(encounter_id);
    if (!entity) return 1.0f;

    auto* enc = entity->getComponent<components::MythBossEncounter>();
    return enc ? enc->difficulty : 1.0f;
}

std::string MythBossSystem::getEncounterMythId(const std::string& encounter_id) const {
    auto* entity = world_->getEntity(encounter_id);
    if (!entity) return "";

    auto* enc = entity->getComponent<components::MythBossEncounter>();
    return enc ? enc->myth_id : "";
}

std::string MythBossSystem::getBossTypeName(int type_index) {
    switch (type_index) {
        case 0: return "Guardian";
        case 1: return "Destroyer";
        case 2: return "Phantom";
        case 3: return "Colossus";
        case 4: return "Mirage";
        default: return "Unknown";
    }
}

} // namespace systems
} // namespace atlas
