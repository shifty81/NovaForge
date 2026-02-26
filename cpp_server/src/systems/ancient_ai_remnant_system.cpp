#include "systems/ancient_ai_remnant_system.h"
#include "ecs/world.h"
#include <algorithm>
#include <memory>

namespace atlas {
namespace systems {

static constexpr float BASE_REMNANT_HP = 1500.0f;
static constexpr float HP_PER_TIER = 500.0f;
static constexpr float BASE_DAMAGE = 50.0f;
static constexpr float DAMAGE_PER_TIER = 25.0f;

AncientAIRemnantSystem::AncientAIRemnantSystem(ecs::World* world)
    : System(world) {
}

void AncientAIRemnantSystem::update(float delta_time) {
    auto entities = world_->getEntities<components::AncientAIRemnant>();
    for (auto* entity : entities) {
        auto* remnant = entity->getComponent<components::AncientAIRemnant>();
        if (!remnant || !remnant->active) continue;

        remnant->active_time += delta_time;

        if (remnant->isExpired()) {
            remnant->active = false;
        }
    }
}

std::string AncientAIRemnantSystem::spawnRemnant(const std::string& site_id, int tier) {
    int clamped_tier = std::max(1, std::min(5, tier));

    std::string remnant_id = "remnant_" + std::to_string(++remnant_counter_);
    auto* entity = world_->createEntity(remnant_id);
    auto comp = std::make_unique<components::AncientAIRemnant>();

    comp->remnant_id = remnant_id;
    comp->site_id = site_id;
    comp->tier = clamped_tier;

    // Map tier to remnant type
    switch (clamped_tier) {
        case 1: comp->remnant_type = components::AncientAIRemnant::RemnantType::Sentinel; break;
        case 2: comp->remnant_type = components::AncientAIRemnant::RemnantType::Swarm; break;
        case 3: comp->remnant_type = components::AncientAIRemnant::RemnantType::Construct; break;
        case 4: comp->remnant_type = components::AncientAIRemnant::RemnantType::Warden; break;
        case 5: comp->remnant_type = components::AncientAIRemnant::RemnantType::Leviathan; break;
        default: comp->remnant_type = components::AncientAIRemnant::RemnantType::Sentinel; break;
    }

    // Scale stats by tier
    comp->difficulty = static_cast<float>(clamped_tier);
    comp->hit_points = BASE_REMNANT_HP + HP_PER_TIER * static_cast<float>(clamped_tier);
    comp->damage_output = BASE_DAMAGE + DAMAGE_PER_TIER * static_cast<float>(clamped_tier);
    comp->recommended_fleet_size = std::max(1, clamped_tier * 2);

    // Generate reward entries proportional to tier
    for (int i = 0; i < clamped_tier; ++i) {
        components::AncientAIRemnant::RewardEntry reward;
        reward.item_id = "ancient_salvage_" + std::to_string(i + 1);
        reward.drop_chance = std::min(1.0f, 0.3f + 0.1f * static_cast<float>(clamped_tier));
        reward.quantity = clamped_tier;
        comp->rewards.push_back(reward);
    }

    comp->active = true;
    comp->active_time = 0.0f;

    entity->addComponent(std::move(comp));
    return remnant_id;
}

bool AncientAIRemnantSystem::isRemnantActive(const std::string& remnant_id) const {
    auto* entity = world_->getEntity(remnant_id);
    if (!entity) return false;
    auto* remnant = entity->getComponent<components::AncientAIRemnant>();
    return remnant && remnant->isActive();
}

bool AncientAIRemnantSystem::defeatRemnant(const std::string& remnant_id) {
    auto* entity = world_->getEntity(remnant_id);
    if (!entity) return false;
    auto* remnant = entity->getComponent<components::AncientAIRemnant>();
    if (!remnant || !remnant->active) return false;
    remnant->active = false;
    remnant->defeated = true;
    return true;
}

int AncientAIRemnantSystem::getActiveRemnantCount() const {
    int count = 0;
    auto entities = world_->getEntities<components::AncientAIRemnant>();
    for (auto* entity : entities) {
        auto* remnant = entity->getComponent<components::AncientAIRemnant>();
        if (remnant && remnant->isActive()) count++;
    }
    return count;
}

float AncientAIRemnantSystem::getRemnantDifficulty(const std::string& remnant_id) const {
    auto* entity = world_->getEntity(remnant_id);
    if (!entity) return 1.0f;
    auto* remnant = entity->getComponent<components::AncientAIRemnant>();
    return remnant ? remnant->difficulty : 1.0f;
}

std::string AncientAIRemnantSystem::getRemnantSiteId(const std::string& remnant_id) const {
    auto* entity = world_->getEntity(remnant_id);
    if (!entity) return "";
    auto* remnant = entity->getComponent<components::AncientAIRemnant>();
    return remnant ? remnant->site_id : "";
}

std::string AncientAIRemnantSystem::getRemnantTypeName(int type_index) {
    switch (type_index) {
        case 0: return "Sentinel";
        case 1: return "Swarm";
        case 2: return "Construct";
        case 3: return "Warden";
        case 4: return "Leviathan";
        default: return "Unknown";
    }
}

} // namespace systems
} // namespace atlas
