#include "systems/loot_distribution_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>
#include <numeric>

namespace atlas {
namespace systems {

LootDistributionSystem::LootDistributionSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

// ---------------------------------------------------------------------------
// Tick
// ---------------------------------------------------------------------------

void LootDistributionSystem::updateComponent(ecs::Entity& /*entity*/,
                                              components::LootDistribution& comp,
                                              float delta_time) {
    if (!comp.active) return;
    comp.elapsed += delta_time;
}

// ---------------------------------------------------------------------------
// Lifecycle
// ---------------------------------------------------------------------------

bool LootDistributionSystem::initialize(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::LootDistribution>();
    entity->addComponent(std::move(comp));
    return true;
}

// ---------------------------------------------------------------------------
// Distribution control
// ---------------------------------------------------------------------------

bool LootDistributionSystem::openDistribution(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (comp->state != components::LootDistribution::State::Idle) return false;
    comp->state = components::LootDistribution::State::Open;
    comp->participants.clear();
    comp->items.clear();
    comp->isk_pool = 0.0f;
    return true;
}

bool LootDistributionSystem::distribute(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (comp->state != components::LootDistribution::State::Open) return false;
    if (comp->participants.empty()) return false;

    // Calculate total damage across all participants
    float total_damage = 0.0f;
    for (const auto& p : comp->participants) {
        total_damage += p.damage_dealt;
    }

    // Assign proportional ISK shares
    if (total_damage > 0.0f) {
        for (auto& p : comp->participants) {
            p.isk_share = comp->isk_pool * (p.damage_dealt / total_damage);
        }
    } else {
        // Equal split when no damage data
        float equal_share = (comp->participants.empty())
            ? 0.0f
            : comp->isk_pool / static_cast<float>(comp->participants.size());
        for (auto& p : comp->participants) {
            p.isk_share = equal_share;
        }
    }

    // Assign items round-robin by descending damage contribution
    if (!comp->items.empty() && !comp->participants.empty()) {
        // Sort participants by damage dealt descending for item priority
        std::vector<std::size_t> order;
        order.reserve(comp->participants.size());
        for (std::size_t i = 0; i < comp->participants.size(); ++i) {
            order.push_back(i);
        }
        std::sort(order.begin(), order.end(),
            [&](std::size_t a, std::size_t b) {
                return comp->participants[a].damage_dealt >
                       comp->participants[b].damage_dealt;
            });

        std::size_t idx = 0;
        for (auto& item : comp->items) {
            item.assigned_to = comp->participants[order[idx % order.size()]].pilot_id;
            ++idx;
        }
    }

    comp->state = components::LootDistribution::State::Distributed;
    comp->total_distributions++;
    return true;
}

// ---------------------------------------------------------------------------
// Participant management
// ---------------------------------------------------------------------------

bool LootDistributionSystem::addParticipant(const std::string& entity_id,
                                             const std::string& pilot_id,
                                             float damage_dealt) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (comp->state != components::LootDistribution::State::Open) return false;
    if (pilot_id.empty() || damage_dealt < 0.0f) return false;
    if (static_cast<int>(comp->participants.size()) >= comp->max_participants) return false;

    for (const auto& p : comp->participants) {
        if (p.pilot_id == pilot_id) return false;
    }

    components::LootDistribution::Participant p;
    p.pilot_id     = pilot_id;
    p.damage_dealt = damage_dealt;
    p.isk_share    = 0.0f;
    comp->participants.push_back(p);
    return true;
}

bool LootDistributionSystem::removeParticipant(const std::string& entity_id,
                                                const std::string& pilot_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (comp->state != components::LootDistribution::State::Open) return false;

    auto it = std::find_if(comp->participants.begin(), comp->participants.end(),
        [&](const components::LootDistribution::Participant& p) {
            return p.pilot_id == pilot_id;
        });
    if (it == comp->participants.end()) return false;
    comp->participants.erase(it);
    return true;
}

// ---------------------------------------------------------------------------
// Pool management
// ---------------------------------------------------------------------------

bool LootDistributionSystem::setIskPool(const std::string& entity_id,
                                         float amount) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (comp->state != components::LootDistribution::State::Open) return false;
    if (amount < 0.0f) return false;
    comp->isk_pool = amount;
    return true;
}

bool LootDistributionSystem::addItem(const std::string& entity_id,
                                      const std::string& item_id,
                                      const std::string& item_name,
                                      int quantity) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (comp->state != components::LootDistribution::State::Open) return false;
    if (item_id.empty() || quantity <= 0) return false;
    if (static_cast<int>(comp->items.size()) >= comp->max_items) return false;

    for (const auto& i : comp->items) {
        if (i.item_id == item_id) return false;
    }

    components::LootDistribution::LootItem item;
    item.item_id    = item_id;
    item.item_name  = item_name;
    item.quantity   = quantity;
    comp->items.push_back(item);
    return true;
}

// ---------------------------------------------------------------------------
// Queries
// ---------------------------------------------------------------------------

components::LootDistribution::State
LootDistributionSystem::getState(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->state : components::LootDistribution::State::Idle;
}

int LootDistributionSystem::getParticipantCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? static_cast<int>(comp->participants.size()) : 0;
}

int LootDistributionSystem::getItemCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? static_cast<int>(comp->items.size()) : 0;
}

float LootDistributionSystem::getIskPool(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->isk_pool : 0.0f;
}

float LootDistributionSystem::getParticipantShare(const std::string& entity_id,
                                                    const std::string& pilot_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0.0f;
    for (const auto& p : comp->participants) {
        if (p.pilot_id == pilot_id) return p.isk_share;
    }
    return 0.0f;
}

int LootDistributionSystem::getTotalDistributions(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_distributions : 0;
}

} // namespace systems
} // namespace atlas
